// models/SuspiciousActivity.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const Player = require('./Player');

const SuspiciousActivity = sequelize.define('SuspiciousActivity', {
  id: {
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
  activityData: {
    type: DataTypes.JSON,
  },
  reason: {
    type: DataTypes.STRING,
  },
  timestamp: {
    type: DataTypes.DATE,
    defaultValue: DataTypes.NOW,
  },
});

Player.hasMany(SuspiciousActivity, { foreignKey: 'playerId' });
SuspiciousActivity.belongsTo(Player, { foreignKey: 'playerId' });

module.exports = SuspiciousActivity;
