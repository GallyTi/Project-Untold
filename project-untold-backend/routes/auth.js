// routes/auth.js
const express = require('express');
const router = express.Router();
const authController = require('../controllers/authController');

//router.post('/register', authController.register);
//router.post('/login', authController.login);
router.post('/eos-login', authController.eosLogin);


module.exports = router;