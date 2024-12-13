// models/Inventory.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const Character = require('./Character');
const Item = require('./Item');

const Inventory = sequelize.define('Inventory', {
  inventoryId: {
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
  itemId: {
    type: DataTypes.INTEGER,
    references: {
      model: Item,
      key: 'itemId',
    },
  },
  quantity: {
    type: DataTypes.INTEGER,
    defaultValue: 1,
  },
});

Character.hasMany(Inventory, { foreignKey: 'characterId' });
Inventory.belongsTo(Character, { foreignKey: 'characterId' });

Item.hasMany(Inventory, { foreignKey: 'itemId' });
Inventory.belongsTo(Item, { foreignKey: 'itemId' });

module.exports = Inventory;