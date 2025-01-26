// controllers/inventoryController.js
const Inventory = require('../models/Inventory');
const Character = require('../models/Character');
const Item = require('../models/Item');
const Stats = require('../models/Stats');
const BuffDebuff = require('../models/BuffDebuff');

exports.getInventory = async (req, res) => {
  try {
    const character = await Character.findOne({ where: { playerId: req.playerId } });
    if (!character) return res.status(404).json({ message: 'Postava nenájdená.' });

    const inventoryItems = await Inventory.findAll({
      where: { characterId: character.characterId },
      include: [Item],
    });
    res.json(inventoryItems);
  } catch (error) {
    res.status(500).json({ error: 'Chyba pri získavaní inventára.' });
  }
  
};

exports.useItem = async (req, res) => {
  const { itemId } = req.body;

  try {
    const character = await Character.findOne({ where: { playerId: req.playerId } });
    if (!character) return res.status(404).json({ message: 'Postava nenájdená.' });

    const inventoryItem = await Inventory.findOne({
      where: {
        characterId: character.characterId,
        itemId,
      },
    });

    if (!inventoryItem || inventoryItem.quantity < 1) {
      return res.status(404).json({ message: 'Predmet nie je v inventári.' });
    }

    // Zníženie množstva predmetu
    if (inventoryItem.quantity > 1) {
      await inventoryItem.decrement('quantity');
    } else {
      await inventoryItem.destroy();
    }

    // Aplikovanie buffov/debuffov predmetu
    const item = await Item.findByPk(itemId, {
      include: [BuffDebuff],
    });

    const stats = await Stats.findOne({ where: { characterId: character.characterId } });

    for (let buffDebuff of item.BuffDebuffs) {
      let valueChange = buffDebuff.valueChange;

      if (buffDebuff.percentageChange) {
        valueChange = (stats[buffDebuff.targetAttribute.toLowerCase()] * valueChange) / 100;
      }

      await stats.increment({ [buffDebuff.targetAttribute.toLowerCase()]: valueChange });
    }

    res.json({ message: `Použil si predmet: ${item.name}` });
  } catch (error) {
    res.status(500).json({ error: 'Chyba pri použití predmetu.' });
  }
};

exports.addItem = async (req, res) => {
  try {
    const { itemId, quantity } = req.body;  // <-- z JSONa: { "itemId": 123, "quantity": 5 }

    // Najskôr nájdeme postavu (Character) podľa req.playerId (z tokenu)
    const character = await Character.findOne({ where: { playerId: req.playerId } });
    if (!character) {
      return res.status(404).json({ message: 'Postava nenájdená.' });
    }

    // Skúsime či item už existuje v inventári
    let inventoryItem = await Inventory.findOne({
      where: {
        characterId: character.characterId,
        itemId: itemId
      }
    });

    if (inventoryItem) {
      // Ak existuje, zvyšime quantity
      await inventoryItem.increment('quantity', { by: quantity });
    } else {
      // Ak neexistuje, vytvoríme nový záznam
      await Inventory.create({
        characterId: character.characterId,
        itemId: itemId,
        quantity: quantity
      });
    }

    return res.status(200).json({ message: 'Item bol pridaný do inventára.' });

  } catch (error) {
    console.error('Chyba pri pridávaní itemu:', error);
    return res.status(500).json({ error: 'Nepodarilo sa pridať item do inventára.' });
  }
};
