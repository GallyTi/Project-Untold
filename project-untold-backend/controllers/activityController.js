// controllers/activityController.js
const Activity = require('../models/Activity');
const sequelize = require('../config/database');
const { Op } = require('sequelize');
const SuspiciousActivity = require('../models/SuspiciousActivity');
const { sendUserWarning } = require('../services/notificationService');

exports.submitActivities = async (req, res) => {
    const activities = req.body;

    try {
        for (const activity of activities) {
            const {
                date, steps, calories, sleepScore, totalSleepTime,
                deepSleepTime, remSleepTime, lightSleepTime, awakeTime
            } = activity;

            // Use upsert to update or create the activity
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
                }
            });

            if (created) {
                console.log(`Created new activity for date ${date} and player ${req.playerId}`);
            } else {
                console.log(`Updated existing activity for date ${date} and player ${req.playerId}`);
            }
        }

        res.status(200).json({ message: 'Data has been successfully saved.' });
    } catch (error) {
        console.error('Error saving activities:', error);
        res.status(500).json({ error: 'Error saving data.' });
    }
};

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

exports.getAggregatedActivities = async (req, res) => {
    try {
        const activities = await Activity.findAll({
            where: { playerId: req.playerId },
            attributes: [
                [sequelize.fn('SUM', sequelize.col('stepCount')), 'totalSteps'],
                // Include other aggregations if needed
            ],
        });

        res.json(activities);
    } catch (error) {
        res.status(500).json({ error: 'Error fetching aggregated activities.' });
    }
};