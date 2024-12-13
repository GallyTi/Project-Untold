// controllers/achievementController.js
const Achievement = require('../models/Achievement');
const PlayerAchievement = require('../models/PlayerAchievement');
const Player = require('../models/Player');

exports.getAchievements = async (req, res) => {
  try {
    const playerAchievements = await PlayerAchievement.findAll({
      where: { playerId: req.playerId },
      include: [Achievement],
    });

    res.json(playerAchievements);
  } catch (error) {
    res.status(500).json({ error: 'Chyba pri získavaní achievementov.' });
  }
};

exports.checkAchievements = async (playerId) => {
  try {
    const player = await Player.findByPk(playerId);
    const achievements = await Achievement.findAll();

    for (let achievement of achievements) {
      // Skontroluj, či hráč už má tento achievement
      const hasAchievement = await PlayerAchievement.findOne({
        where: {
          playerId,
          achievementId: achievement.achievementId,
        },
      });

      if (hasAchievement) continue;

      // Implementuj logiku na overenie podmienok achievementu
      const conditionsMet = checkConditions(player, achievement.conditions);

      if (conditionsMet) {
        // Udelíme achievement hráčovi
        await PlayerAchievement.create({
          playerId,
          achievementId: achievement.achievementId,
        });

        // Udelíme odmeny
        // Napríklad XP alebo predmety podľa achievement.rewards

        console.log(`Hráč ${player.name} získal achievement: ${achievement.name}`);
      }
    }
  } catch (error) {
    console.error('Chyba pri overovaní achievementov:', error);
  }
};

function checkConditions(player, conditions) {
  // Implementuj logiku na overenie, či hráč spĺňa podmienky
  // Táto funkcia by mala vrátiť true, ak sú podmienky splnené, inak false
  // Prispôsob podľa formátu tvojich podmienok
  return false;
}