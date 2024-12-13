// models/Player.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');
const bcrypt = require('bcryptjs');

const Player = sequelize.define('Player', {
  playerId: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true,
  },
  eosAccountId: {
    type: DataTypes.STRING,
    unique: true,
    allowNull: true,
  },
  eosProductUserId: {
    type: DataTypes.STRING,
    unique: true,
    allowNull: true,
  },
  name: { 
    type: DataTypes.STRING, 
    allowNull: false 
  },
  email: { 
    type: DataTypes.STRING, 
    allowNull: false, 
    unique: true,
    validate: {
      isEmail: true,
    },
  },
  password: { 
    type: DataTypes.STRING, 
    allowNull: false 
  },
  gender: {
    type: DataTypes.ENUM(
      'Muž', 'Muz', 'Male',
      'Žena', 'Zena', 'Female',
      'Neuvádza', 'Neuvadza', 'N/A'
    ),
    defaultValue: 'Neuvádza',
    allowNull: false
  },
  age: { 
    type: DataTypes.INTEGER 
  },
  level: { 
    type: DataTypes.INTEGER, 
    defaultValue: 1 
  },
  xp: { 
    type: DataTypes.INTEGER, 
    defaultValue: 0 
  },
}, {
  tableName: 'players', // Explicitné nastavenie názvu tabuľky
});

// Hash password before saving the player
Player.beforeCreate(async (player) => {
  if (player.password) {
    player.password = await bcrypt.hash(player.password, 10);
  }
});

// Hash password if it has changed before updating
Player.beforeUpdate(async (player) => {
  if (player.changed('password')) {
    player.password = await bcrypt.hash(player.password, 10);
  }
});

module.exports = Player;
