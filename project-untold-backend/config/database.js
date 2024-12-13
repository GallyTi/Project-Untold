require('dotenv').config();
const { Sequelize } = require('sequelize');

// Nastavenie pripojenia k databáze
const sequelize = new Sequelize(process.env.DATABASE_URL, {
  dialect: 'postgres',
  // Zakomentované detailné logovanie
  logging: false, // Nastavenie na `false` vypne všetky SQL logy
});

// Funkcia na pripojenie s opakovaním
const connectWithRetry = () => {
  sequelize
    .authenticate()
    .then(() => console.log('Database connected'))
    .catch((err) => {
      console.error('Database connection failed, retrying...', err.message);
      setTimeout(connectWithRetry, 5000); // Opakovať po 5 sekundách
    });
};

// Spustenie pripojenia k databáze
connectWithRetry();

// Export Sequelize inštancie na použitie inde
module.exports = sequelize;
