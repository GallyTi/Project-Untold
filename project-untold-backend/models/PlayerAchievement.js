// models/PlayerAchievement.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const Player = require('./Player');
const Achievement = require('./Achievement');

const PlayerAchievement = sequelize.define('PlayerAchievement', {
  playerAchievementId: {
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
  achievementId: {
    type: DataTypes.INTEGER,
    references: {
      model: Achievement,
      key: 'achievementId',
    },
  },
  dateObtained: {
    type: DataTypes.DATE,
    defaultValue: DataTypes.NOW,
  },
});

Player.belongsToMany(Achievement, {
  through: PlayerAchievement,
  foreignKey: 'playerId',
});
Achievement.belongsToMany(Player, {
  through: PlayerAchievement,
  foreignKey: 'achievementId',
});

module.exports = PlayerAchievement;