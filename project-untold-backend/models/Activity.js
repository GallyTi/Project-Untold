// models/Activity.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const Player = require('./Player');

const Activity = sequelize.define('Activity', {
  activityId: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true,
  },
  playerId: {
    type: DataTypes.INTEGER,
    references: {
      model: Player,
      key: 'playerId',
    },
  },
    date: { type: DataTypes.DATEONLY, allowNull: false },
    stepCount: { type: DataTypes.INTEGER, defaultValue: 0 },
    calories: { type: DataTypes.FLOAT, defaultValue: 0 },
    sleepScore: { type: DataTypes.FLOAT, defaultValue: 0 },
    totalSleepTime: { type: DataTypes.FLOAT, defaultValue: 0 },
    deepSleepTime: { type: DataTypes.FLOAT, defaultValue: 0 },
    remSleepTime: { type: DataTypes.FLOAT, defaultValue: 0 },
    lightSleepTime: { type: DataTypes.FLOAT, defaultValue: 0 },
    awakeTime: { type: DataTypes.FLOAT, defaultValue: 0 },
  }, {
  indexes: [
    {
        unique: true,
        fields: ['playerId', 'date']
    }
  ]
});

Player.hasMany(Activity, { foreignKey: 'playerId' });
Activity.belongsTo(Player, { foreignKey: 'playerId' });

module.exports = Activity;