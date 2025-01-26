// routes/auth.js
const express = require('express');
const router = express.Router();
const authController = require('../controllers/authController');
const authenticateToken = require('../middleware/auth');

// Pridaj sem testovací endpoint
router.get('/test', authenticateToken, (req, res) => {
    return res.json({
      message: 'Token je platný!',
      playerId: req.playerId,
    });
  });

//router.post('/register', authController.register);
//router.post('/login', authController.login);
router.post('/eos-login', authController.eosLogin);


module.exports = router;