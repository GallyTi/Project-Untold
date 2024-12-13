// models/Stats.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const Character = require('./Character');

const Stats = sequelize.define('Stats', {
  statsId: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true,
  },
  characterId: {
    type: DataTypes.INTEGER,
    references: {
      model: Character,
      key: 'characterId',
    },
  },
  strength: { type: DataTypes.INTEGER, defaultValue: 0 },
  stamina: { type: DataTypes.INTEGER, defaultValue: 0 },
  health: { type: DataTypes.INTEGER, defaultValue: 100 },
  intelligence: { type: DataTypes.INTEGER, defaultValue: 0 },
  mana: { type: DataTypes.INTEGER, defaultValue: 0 },
});

Character.hasOne(Stats, { foreignKey: 'characterId' });
Stats.belongsTo(Character, { foreignKey: 'characterId' });

module.exports = Stats;
