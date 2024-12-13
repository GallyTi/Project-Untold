// routes/quests.js
const express = require('express');
const router = express.Router();
const questController = require('../controllers/questController');
const authenticateToken = require('../middleware/auth');

router.get('/', authenticateToken, questController.getQuests);
router.post('/accept', authenticateToken, questController.acceptQuest);
router.post('/complete', authenticateToken, questController.completeQuest);

module.exports = router;