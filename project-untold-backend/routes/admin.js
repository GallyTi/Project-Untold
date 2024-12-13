// routes/admin.js
const express = require('express');
const router = express.Router();
const { sequelize } = require('../config/database');
const authenticateAdminToken = require('../middleware/adminAuth');

router.post('/reset-data', authenticateAdminToken, async (req, res) => {
  try {
    await sequelize.query('TRUNCATE TABLE "Activities" RESTART IDENTITY CASCADE');
    res.json({ message: 'Data reset successfully.' });
  } catch (error) {
    res.status(500).json({ error: 'Error resetting data.' });
  }
});

module.exports = router;
