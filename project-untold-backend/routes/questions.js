// routes/questions.js
const express = require('express');
const router = express.Router();
const questionController = require('../controllers/questionController');
const authenticateToken = require('../middleware/auth');

router.get('/random', authenticateToken, questionController.getRandomQuestion);
router.post('/answer', authenticateToken, questionController.submitAnswer);

module.exports = router;