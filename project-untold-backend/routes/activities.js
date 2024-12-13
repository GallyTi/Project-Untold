// routes/activities.js
const express = require('express');
const router = express.Router();
const activityController = require('../controllers/activityController');
const authenticateToken = require('../middleware/auth');

router.post('/', authenticateToken, activityController.submitActivities);
router.get('/all', authenticateToken, activityController.getAllActivities);
router.get('/aggregate', authenticateToken, activityController.getAggregatedActivities);


module.exports = router;