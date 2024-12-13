// models/PlayerQuest.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const Player = require('./Player');
const Quest = require('./Quest');

const PlayerQuest = sequelize.define('PlayerQuest', {
  playerQuestId: {
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
  questId: {
    type: DataTypes.INTEGER,
    references: {
      model: Quest,
      key: 'questId',
    },
  },
  status: {
    type: DataTypes.ENUM('In Progress', 'Completed'),
    defaultValue: 'In Progress',
  },
});

Player.belongsToMany(Quest, { through: PlayerQuest, foreignKey: 'playerId' });
Quest.belongsToMany(Player, { through: PlayerQuest, foreignKey: 'questId' });

module.exports = PlayerQuest;