// models/PlayerAnswer.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const Player = require('./Player');
const IntelligenceQuestion = require('./IntelligenceQuestion');

const PlayerAnswer = sequelize.define('PlayerAnswer', {
  answerId: {
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
  questionId: {
    type: DataTypes.INTEGER,
    references: {
      model: IntelligenceQuestion,
      key: 'questionId',
    },
  },
  isCorrect: {
    type: DataTypes.BOOLEAN,
    allowNull: false,
  },
  answeredAt: {
    type: DataTypes.DATE,
    defaultValue: DataTypes.NOW,
  },
});

Player.hasMany(PlayerAnswer, { foreignKey: 'playerId' });
PlayerAnswer.belongsTo(Player, { foreignKey: 'playerId' });

IntelligenceQuestion.hasMany(PlayerAnswer, { foreignKey: 'questionId' });
PlayerAnswer.belongsTo(IntelligenceQuestion, { foreignKey: 'questionId' });

module.exports = PlayerAnswer;