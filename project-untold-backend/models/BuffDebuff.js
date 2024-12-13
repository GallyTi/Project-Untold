// models/BuffDebuff.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');

const BuffDebuff = sequelize.define('BuffDebuff', {
  buffDebuffId: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true,
  },
  name: { type: DataTypes.STRING, allowNull: false },
  description: { type: DataTypes.TEXT },
  type: { type: DataTypes.ENUM('Buff', 'Debuff'), allowNull: false },
  targetAttribute: { type: DataTypes.STRING, allowNull: false }, // Napr. 'Strength', 'Intelligence'
  valueChange: { type: DataTypes.FLOAT, allowNull: false },
  percentageChange: { type: DataTypes.BOOLEAN, defaultValue: false },
  duration: { type: DataTypes.INTEGER }, // Trvanie v sekundách
});

module.exports = BuffDebuff;