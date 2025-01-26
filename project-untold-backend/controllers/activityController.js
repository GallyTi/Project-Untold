// project-untold-backend/controllers/activityController.js

const Activity = require('../models/Activity');
const sequelize = require('../config/database');
const { Op } = require('sequelize');

// Jednoduchá in-memory cache (napr. držíme len posledných 10) s kľúčom = playerId
// a hodnotou = pole detailov o aktivity
// V reálnom prostredí by ste skôr použili Redis alebo iné riešenie
const activityCache = {};

/**
 * Uloží (upsertne) aktivity. Všetko v rámci DB transakcie.
 */
exports.submitActivities = async (req, res) => {
  // Vytvor transakciu
  const t = await sequelize.transaction();
  try {
    const activities = req.body;
    // Pre každý activity objekt z req.body
    for (const activity of activities) {
      const {
        date,
        steps,
        calories,
        sleepScore,
        totalSleepTime,
        deepSleepTime,
        remSleepTime,
        lightSleepTime,
        awakeTime
      } = activity;

      // DB upsert s transakciou
      const [activityRecord, created] = await Activity.upsert({
        playerId: req.playerId,
        date,
        stepCount: steps,
        calories,
        sleepScore,
        totalSleepTime,
        deepSleepTime,
        remSleepTime,
        lightSleepTime,
        awakeTime,
      }, {
        where: {
          playerId: req.playerId,
          date: date,
        },
        transaction: t, // dôležité
      });

      if (created) {
        console.log(`Created new activity for date ${date} / player ${req.playerId}`);
      } else {
        console.log(`Updated existing activity for date ${date} / player ${req.playerId}`);
      }

      // Uložíme do našej malej in-memory cache
      // In-memory: activityCache[playerId] = pole
      if (!activityCache[req.playerId]) {
        activityCache[req.playerId] = [];
      }
      activityCache[req.playerId].push({
        date,
        steps,
        calories,
        sleepScore,
        totalSleepTime,
        deepSleepTime,
        remSleepTime,
        lightSleepTime,
        awakeTime,
        timestamp: new Date()
      });

      // Udržiavame len posledných 10
      if (activityCache[req.playerId].length > 10) {
        activityCache[req.playerId].shift();
      }
    }

    // Ak všetko OK, commit transakciu
    await t.commit();
    return res.status(200).json({ message: 'Data has been successfully saved.' });

  } catch (error) {
    // V prípade chyby => rollback
    await t.rollback();
    console.error('Error saving activities (transaction rollback):', error);
    return res.status(500).json({ error: 'Error saving data.' });
  }
};

/**
 * Získanie všetkých aktivít pre daného hráča
 */
exports.getAllActivities = async (req, res) => {
  try {
    const activities = await Activity.findAll({
      where: { playerId: req.playerId },
      order: [['date', 'ASC']],
    });
    res.json(activities);
  } catch (error) {
    res.status(500).json({ error: 'Error fetching activities.' });
  }
};

/**
 * Agregované dáta – sum(steps), ...
 */
exports.getAggregatedActivities = async (req, res) => {
  try {
    const result = await Activity.findAll({
      where: { playerId: req.playerId },
      attributes: [
        [sequelize.fn('SUM', sequelize.col('stepCount')), 'totalSteps'],
        // Prípadne ďalšie agr. funkcie
      ],
    });
    // result je pole, tak v 0 bude objekt s totalSteps
    res.json(result);
  } catch (error) {
    res.status(500).json({ error: 'Error fetching aggregated activities.' });
  }
};
