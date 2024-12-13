// controllers/authController.js

const bcrypt = require('bcryptjs'); // Pre hashovanie hesiel
const jwkToPem = require('jwk-to-pem');
const jwt = require('jsonwebtoken');
const axios = require('axios');
const Player = require('../models/Player');

exports.register = async (req, res) => {
  try {
    const { name, email, password } = req.body;

    // Overenie vstupov
    if (!name || !email || !password) {
      return res.status(400).json({ message: 'Meno, email a heslo sú povinné.' });
    }

    // Skontrolovať, či užívateľ už existuje
    let existingPlayer = await Player.findOne({ where: { email } });
    if (existingPlayer) {
      return res.status(400).json({ message: 'Email je už používaný.' });
    }

    // Hashovanie hesla
    const hashedPassword = await bcrypt.hash(password, 10);

    // Vytvorenie hráča
    let player = await Player.create({
      name,
      email,
      password: hashedPassword,
    });

    // Generovanie JWT tokenu
    const token = jwt.sign({ id: player.playerId }, process.env.JWT_SECRET, { expiresIn: '1h' });

    res.json({ message: 'Registrácia úspešná.', token });
  } catch (error) {
    console.error('Chyba pri registrácii:', error.message);
    res.status(500).json({ message: 'Interná chyba servera.' });
  }
};

exports.login = async (req, res) => {
  try {
    const { email, password } = req.body;

    // Overenie vstupov
    if (!email || !password) {
      return res.status(400).json({ message: 'Email a heslo sú povinné.' });
    }

    // Nájsť hráča
    let player = await Player.findOne({ where: { email } });
    if (!player) {
      return res.status(401).json({ message: 'Nesprávny email alebo heslo.' });
    }

    // Porovnať heslo
    const isMatch = await bcrypt.compare(password, player.password);
    if (!isMatch) {
      return res.status(401).json({ message: 'Nesprávny email alebo heslo.' });
    }

    // Generovanie JWT tokenu
    const token = jwt.sign({ id: player.playerId }, process.env.JWT_SECRET, { expiresIn: '1h' });

    res.json({ message: 'Prihlásenie úspešné.', token });
  } catch (error) {
    console.error('Chyba pri prihlasovaní:', error.message);
    res.status(500).json({ message: 'Interná chyba servera.' });
  }
};

exports.eosLogin = async (req, res) => {
  const { authToken, accountId, productUserId } = req.body;

  if (!authToken || !accountId) {
      return res.status(400).json({ message: 'AuthToken and AccountId are required.' });
  }

  try {
      // Use the updated verifyEOSToken function
      const isValid = await verifyEOSToken(authToken, accountId);
      if (!isValid) {
          return res.status(401).json({ message: 'Invalid AuthToken or AccountId.' });
      }

      // Find or create the player in the database
      let player = await Player.findOne({ where: { eosAccountId: accountId } });
      if (!player) {
          player = await Player.create({
              name: 'New Player',
              email: `${accountId}@epicgames.com`,
              password: '', // EOS players don't have a password
              eosAccountId: accountId,
              eosProductUserId: productUserId, // Store PUID
          });
      } else if (!player.eosProductUserId) {
          // Update PUID if not already set
          player.eosProductUserId = productUserId;
          await player.save();
      }

      // Generate JWT token
      const token = jwt.sign({ id: player.playerId }, process.env.JWT_SECRET, { expiresIn: '1h' });

      if (!token) {
          console.error('JWT token generation failed');
          return res.status(500).json({ message: 'Token generation failed' });
      }

      res.json({ message: 'EOS Login successful.', token });
  } catch (error) {
      console.error('Error during EOS login:', error.message);
      res.status(500).json({ message: 'Internal server error.' });
  }
};

// Funkcia na overenie EOS tokenu
/*
async function verifyEOSToken(authToken, puid) {
  try {
      // Fetch the EOS public keys
      const publicKeys = await getEOSPublicKeys();

      // Decode the token header to get the 'kid' (Key ID)
      const decodedHeader = jwt.decode(authToken, { complete: true });
      if (!decodedHeader) {
          console.error('Invalid AuthToken format.');
          return false;
      }

      const kid = decodedHeader.header.kid;

      // Find the corresponding public key
      const jwk = publicKeys.find(key => key.kid === kid);
      if (!jwk) {
          console.error('Public key not found for kid:', kid);
          return false;
      }

      // Convert JWK to PEM format
      const publicKey = jwkToPem(jwk);

      // Verify the token
      const decodedToken = jwt.verify(authToken, publicKey, { algorithms: ['RS256'] });

      console.log('Decoded Token:', decodedToken);

      const sub = decodedToken.sub;

      console.log('Received sub from decoded token:', sub);
        console.log('Provided accountId:', accountId);

        return sub === accountId;
    } catch (error) {
        console.error('Error verifying EOS token:', error);
        return false;
    }
}*/

async function verifyEOSToken(authToken, accountId) {
  try {
      const publicKeys = await getEOSPublicKeys();
      const decodedHeader = jwt.decode(authToken, { complete: true });
      const kid = decodedHeader.header.kid;
      const jwk = publicKeys.find((key) => key.kid === kid);
      const publicKey = jwkToPem(jwk);

      const decodedToken = jwt.verify(authToken, publicKey, { algorithms: ['RS256'] });
      console.log('Decoded Token:', decodedToken);

      // Tu porovnávajte sub s accountId
      if (decodedToken.sub !== accountId) {
          console.error('Sub does not match accountId');
          return false;
      }

      return true;
  } catch (error) {
      console.error('Error verifying EOS token:', error);
      return false;
  }
}



// Function to fetch EOS public keys
async function getEOSPublicKeys() {
  try {
      const response = await axios.get('https://api.epicgames.dev/epic/oauth/v1/.well-known/jwks.json');
      return response.data.keys;
  } catch (error) {
      console.error('Error fetching EOS public keys:', error);
      throw error;
  }
}



async function verifySteamToken(authToken, puid) {
  // Overenie Steam tokenu
}

async function verifyGoogleToken(authToken, puid) {
  // Overenie Google tokenu
}