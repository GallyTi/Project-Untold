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