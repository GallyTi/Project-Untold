// models/Item.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const BuffDebuff = require('./BuffDebuff');

const Item = sequelize.define('Item', {
  itemId: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true,
  },
  name: { type: DataTypes.STRING, allowNull: false },
  description: { type: DataTypes.TEXT },
  type: { type: DataTypes.STRING, allowNull: false }, // Napr. 'Weapon', 'Armor', 'Potion'
  value: { type: DataTypes.INTEGER, defaultValue: 0 },
});

// Vzťah M:N medzi Item a BuffDebuff
Item.belongsToMany(BuffDebuff, {
  through: 'ItemBuffs',
  foreignKey: 'itemId',
});
BuffDebuff.belongsToMany(Item, {
  through: 'ItemBuffs',
  foreignKey: 'buffDebuffId',
});

module.exports = Item;