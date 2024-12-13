// controllers/questController.js
const Quest = require('../models/Quest');
const PlayerQuest = require('../models/PlayerQuest'); // Potrebujeme definovať tento model
const Character = require('../models/Character');

exports.getQuests = async (req, res) => {
  try {
    const quests = await Quest.findAll();
    res.json(quests);
  } catch (error) {
    res.status(500).json({ error: 'Chyba pri získavaní úloh.' });
  }
};

exports.acceptQuest = async (req, res) => {
  const { questId } = req.body;

  try {
    const quest = await Quest.findByPk(questId);
    if (!quest) {
      return res.status(404).json({ message: 'Úloha neexistuje.' });
    }

    // Skontrolovať, či hráč už prijal úlohu
    const existingPlayerQuest = await PlayerQuest.findOne({
      where: {
        playerId: req.playerId,
        questId,
      },
    });

    if (existingPlayerQuest) {
      return res.status(400).json({ message: 'Úloha už bola prijatá.' });
    }

    // Prijať úlohu
    await PlayerQuest.create({
      playerId: req.playerId,
      questId,
      status: 'In Progress',
    });

    res.json({ message: 'Úloha prijatá.' });
  } catch (error) {
    res.status(500).json({ error: 'Chyba pri prijímaní úlohy.' });
  }
};

exports.completeQuest = async (req, res) => {
  const { questId } = req.body;

  try {
    const playerQuest = await PlayerQuest.findOne({
      where: {
        playerId: req.playerId,
        questId,
        status: 'In Progress',
      },
    });

    if (!playerQuest) {
      return res.status(404).json({ message: 'Úloha nie je v stave prebiehajúca.' });
    }

    // Tu by si mal skontrolovať, či sú splnené podmienky úlohy

    // Aktualizovať stav úlohy na Completed
    await playerQuest.update({ status: 'Completed' });

    // Udelíme hráčovi odmeny
    const quest = await Quest.findByPk(questId);
    const character = await Character.findOne({ where: { playerId: req.playerId } });

    // Udelíme XP
    if (quest.rewards.xp) {
      await character.increment('characterXP', { by: quest.rewards.xp });
      // Kontrola úrovne postavy
      await checkCharacterLevelUp(character);
    }

    // Udelíme predmety
    if (quest.rewards.items) {
      for (let itemId of quest.rewards.items) {
        await Inventory.create({
          characterId: character.characterId,
          itemId,
          quantity: 1,
        });
      }
    }

    res.json({ message: 'Úloha dokončená a odmeny udelené.' });
  } catch (error) {
    res.status(500).json({ error: 'Chyba pri dokončovaní úlohy.' });
  }
};

async function checkCharacterLevelUp(character) {
  const xpForNextLevel = character.characterLevel * 100;
  if (character.characterXP >= xpForNextLevel) {
    await character.update({
      characterLevel: character.characterLevel + 1,
      characterXP: character.characterXP - xpForNextLevel,
    });

    const stats = await Stats.findOne({ where: { characterId: character.characterId } });
    await stats.increment({
      strength: 1,
      stamina: 1,
      health: 10,
      intelligence: 1,
    });

    console.log(`Postava ${character.characterName} dosiahla úroveň ${character.characterLevel}!`);
  }
}