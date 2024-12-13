// routes/achievements.js
const express = require('express');
const router = express.Router();
const achievementController = require('../controllers/achievementController');
const authenticateToken = require('../middleware/auth');

router.get('/', authenticateToken, achievementController.getAchievements);

module.exports = router;