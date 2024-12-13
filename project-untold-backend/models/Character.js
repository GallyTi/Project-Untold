// models/Character.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const Player = require('./Player');

const Character = sequelize.define('Character', {
  characterId: {
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
  characterName: { type: DataTypes.STRING, allowNull: false },
  characterType: { type: DataTypes.STRING, allowNull: false },
  characterLevel: { type: DataTypes.INTEGER, defaultValue: 1 },
  characterXP: { type: DataTypes.INTEGER, defaultValue: 0 },
});

Player.hasMany(Character, { foreignKey: 'playerId' });
Character.belongsTo(Player, { foreignKey: 'playerId' });

module.exports = Character;
