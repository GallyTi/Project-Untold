// controllers/questionController.js
const IntelligenceQuestion = require('../models/IntelligenceQuestion');
const PlayerAnswer = require('../models/PlayerAnswer');
const Character = require('../models/Character');
const Stats = require('../models/Stats');
const { Op } = require('sequelize');

exports.getRandomQuestion = async (req, res) => {
  try {
    // Získanie ID otázok, na ktoré hráč už odpovedal
    const answeredQuestions = await PlayerAnswer.findAll({
      where: { playerId: req.playerId },
      attributes: ['questionId'],
    });
    const answeredQuestionIds = answeredQuestions.map(aq => aq.questionId);

    // Nájde otázku, na ktorú hráč ešte neodpovedal
    const question = await IntelligenceQuestion.findOne({
      where: { questionId: { [Op.notIn]: answeredQuestionIds } },
      order: sequelize.random(), // Náhodná otázka
    });

    if (!question) {
      return res.status(404).json({ message: 'Žiadne ďalšie otázky nie sú dostupné.' });
    }

    res.json({
      questionId: question.questionId,
      questionText: question.questionText,
      options: question.options,
    });
  } catch (error) {
    res.status(500).json({ error: 'Chyba pri získavaní otázky.' });
  }
};

exports.submitAnswer = async (req, res) => {
  const { questionId, answer } = req.body;

  try {
    const question = await IntelligenceQuestion.findByPk(questionId);
    if (!question) {
      return res.status(404).json({ message: 'Otázka neexistuje.' });
    }

    const isCorrect =
      answer.trim().toLowerCase() === question.correctAnswer.trim().toLowerCase();

    // Uloženie odpovede hráča
    await PlayerAnswer.create({
      playerId: req.playerId,
      questionId,
      isCorrect,
    });

    if (isCorrect) {
      // Aktualizácia inteligencie a XP postavy
      const character = await Character.findOne({ where: { playerId: req.playerId } });
      const stats = await Stats.findOne({ where: { characterId: character.characterId } });

      await stats.increment({ intelligence: question.pointValue });

      // Aktualizácia XP postavy
      await character.increment({ characterXP: question.pointValue });

      // Kontrola úrovne postavy
      await checkCharacterLevelUp(character);

      res.json({ message: 'Správna odpoveď!', gainedXP: question.pointValue });
    } else {
      res.json({ message: 'Nesprávna odpoveď.' });
    }
  } catch (error) {
    res.status(500).json({ error: 'Chyba pri spracovaní odpovede.' });
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