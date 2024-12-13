// models/IntelligenceQuestion.js
const { DataTypes } = require('sequelize');
const sequelize = require('../config/database');

const IntelligenceQuestion = sequelize.define('IntelligenceQuestion', {
  questionId: {
    type: DataTypes.INTEGER,
    primaryKey: true,
    autoIncrement: true,
  },
  questionText: {
    type: DataTypes.TEXT,
    allowNull: false,
  },
  options: {
    type: DataTypes.ARRAY(DataTypes.STRING),
  },
  correctAnswer: {
    type: DataTypes.STRING,
    allowNull: false,
  },
  pointValue: {
    type: DataTypes.INTEGER,
    defaultValue: 1,
  },
  category: {
    type: DataTypes.STRING,
  },
}, {
  tableName: 'intelligence_questions', // Explicitly set the table name
});

module.exports = IntelligenceQuestion;