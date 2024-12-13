// models/PlayerPosition.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const Player = require('./Player');

const PlayerPosition = sequelize.define('PlayerPosition', {
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
  positionX: {
    type: DataTypes.FLOAT,
    allowNull: false,
  },
  positionY: {
    type: DataTypes.FLOAT,
    allowNull: false,
  },
  positionZ: {
    type: DataTypes.FLOAT,
    allowNull: false,
  },
  timestamp: {
    type: DataTypes.DATE,
    defaultValue: DataTypes.NOW,
  },
});

Player.hasMany(PlayerPosition, { foreignKey: 'playerId' });
PlayerPosition.belongsTo(Player, { foreignKey: 'playerId' });

module.exports = PlayerPosition;
