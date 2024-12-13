// routes/positions.js
const express = require('express');
const router = express.Router();
const positionController = require('../controllers/positionController');
const authenticateToken = require('../middleware/auth');

router.post('/save', authenticateToken, positionController.savePosition);
router.get('/load', authenticateToken, positionController.loadPosition);

module.exports = router;
