// models/Quest.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');

const Quest = sequelize.define('Quest', {
  questId: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true,
  },
  name: { type: DataTypes.STRING, allowNull: false },
  description: { type: DataTypes.TEXT },
  type: {
    type: DataTypes.ENUM('Daily', 'Weekly', 'Monthly', 'Main'),
    allowNull: false,
  },
  status: {
    type: DataTypes.ENUM('Open', 'In Progress', 'Completed'),
    defaultValue: 'Open',
  },
  rewards: { type: DataTypes.JSON }, // Napr. { xp: 100, items: [itemId1, itemId2] }
  conditions: { type: DataTypes.JSON }, // Definícia podmienok úlohy
});

module.exports = Quest;