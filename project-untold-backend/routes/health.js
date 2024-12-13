// routes/health.js
const express = require('express');
const router = express.Router();
const sequelize = require('../config/database'); // Uisti sa, že cesta je správna

router.get('/', async (req, res) => {
  try {
    // Pokus o jednoduchý dotaz do databázy
    await sequelize.authenticate();
    res.status(200).json({
      status: 'OK',
      database: 'Connected',
      timestamp: new Date().toISOString(),
    });
  } catch (error) {
    console.error('Databázové pripojenie zlyhalo:', error);
    res.status(500).json({
      status: 'Error',
      database: 'Disconnected',
      timestamp: new Date().toISOString(),
    });
  }
});

module.exports = router;
