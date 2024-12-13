// services/notificationService.js
const Player = require('../models/Player');
const emailService = require('./emailService');

const sendUserWarning = async (playerId, message) => {
    const player = await Player.findByPk(playerId);
  
    if (player) {
      await emailService.send({
        to: player.email,
        subject: 'Warning: Suspicious Activity Detected',
        text: message,
      });
    }
};

module.exports = {
  sendUserWarning,
};
