require('dotenv').config();
const express = require('express');
const sequelize = require('./config/database');

const PORT = process.env.PORT || 3000;

const app = express();
app.use(express.json());

// Import modelov
const Player = require('./models/Player');
const Character = require('./models/Character');
const Stats = require('./models/Stats');
const Activity = require('./models/Activity');
const IntelligenceQuestion = require('./models/IntelligenceQuestion');
const PlayerAnswer = require('./models/PlayerAnswer');
const Item = require('./models/Item');
const BuffDebuff = require('./models/BuffDebuff');
const Inventory = require('./models/Inventory');
const Quest = require('./models/Quest');
const Achievement = require('./models/Achievement');
const PlayerAchievement = require('./models/PlayerAchievement');
const PlayerQuest = require('./models/PlayerQuest');

// Definovanie rout
app.use('/auth', require('./routes/auth'));
app.use('/activities', require('./routes/activities'));
app.use('/questions', require('./routes/questions'));
app.use('/inventory', require('./routes/inventory'));
app.use('/quests', require('./routes/quests'));
app.use('/achievements', require('./routes/achievements'));
app.use('/positions', require('./routes/positions'));
app.use('/health', require('./routes/health'));

// Synchronizácia databázy a spustenie servera
sequelize
  .sync() // Bez `alter: true`, aby sa zabránilo nadbytočným zmenám a logom
  .then(() => {
    console.log('Database synchronized');
    app.listen(PORT, '0.0.0.0', () => {
      console.log(`Server running on port ${PORT}`);
    });
  })
  .catch((err) => {
    console.error('Error synchronizing the database:', err);
  });