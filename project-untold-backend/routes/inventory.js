// routes/inventory.js
const express = require('express');
const router = express.Router();
const inventoryController = require('../controllers/inventoryController');
const authenticateToken = require('../middleware/auth');

router.get('/', authenticateToken, inventoryController.getInventory);
router.post('/use-item', authenticateToken, inventoryController.useItem);
router.post('/add-item', authenticateToken, inventoryController.addItem);

module.exports = router;