// controllers/positionController.js
const PlayerPosition = require('../models/PlayerPosition');

exports.savePosition = async (req, res) => {
  const { positionX, positionY, positionZ } = req.body;

  try {
    // Save or update the player's position
    await PlayerPosition.upsert({
      playerId: req.playerId,
      positionX,
      positionY,
      positionZ,
    });

    res.status(200).json({ message: 'Position saved successfully.' });
  } catch (error) {
    res.status(500).json({ error: 'Error saving position.' });
  }
};

exports.loadPosition = async (req, res) => {
  try {
    const position = await PlayerPosition.findOne({ where: { playerId: req.playerId } });

    if (position) {
      res.status(200).json({
        positionX: position.positionX,
        positionY: position.positionY,
        positionZ: position.positionZ,
      });
    } else {
      res.status(404).json({ message: 'Position not found.' });
    }
  } catch (error) {
    res.status(500).json({ error: 'Error loading position.' });
  }
};
