// models/Achievement.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');

const Achievement = sequelize.define('Achievement', {
  achievementId: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true,
  },
  name: { type: DataTypes.STRING, allowNull: false },
  description: { type: DataTypes.TEXT },
  conditions: { type: DataTypes.JSON }, // Definícia podmienok pre dosiahnutie
  rewards: { type: DataTypes.JSON }, // Napr. { xp: 500, items: [itemId] }
});

module.exports = Achievement;