const sequelize = require('./config/database');
const IntelligenceQuestion = require('./models/IntelligenceQuestion');

async function seed() {
  try {
    // Počkajte na pripojenie k databáze
    await connectWithRetry();

    // Váš kód na vkladanie dát
    const questions = [
      {
        questionText: 'Čo je hlavné mesto Slovenska?',
        options: ['Bratislava', 'Košice', 'Žilina', 'Banská Bystrica'],
        correctAnswer: 'Bratislava',
        pointValue: 1,
        category: 'Geografia',
      },
      {
        questionText: 'Koľko planét má slnečná sústava?',
        options: ['7', '8', '9', '10'],
        correctAnswer: '8',
        pointValue: 1,
        category: 'Veda',
      },
      // Ďalšie otázky
    ];

    for (const questionData of questions) {
      await IntelligenceQuestion.findOrCreate({
  where: { questionText: questionData.questionText },
  defaults: questionData,
});
    }

    console.log('Dáta boli úspešne vložené alebo už existovali.');
  } catch (error) {
    console.error('Chyba pri vkladaní dát:', error);
  } finally {
    await sequelize.close();
    process.exit();
  }
}

async function connectWithRetry() {
  let retries = 5;
  while (retries) {
    try {
      await sequelize.authenticate();
      console.log('Databáza pripojená');
      break;
    } catch (err) {
      console.error('Chyba pripojenia k databáze:', err.message);
      retries -= 1;
      console.log(`Opakujem pripojenie o 5 sekúnd... Zostáva pokusov: ${retries}`);
      await new Promise(res => setTimeout(res, 5000));
    }
  }

  if (!retries) {
    throw new Error('Nepodarilo sa pripojiť k databáze po viacerých pokusoch.');
  }
}

seed();