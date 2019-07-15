const deadlineMath = require('./build/Release/deadlinemath');

module.exports = {
  calculateScoop: (height, gensig) => deadlineMath.calculate_scoop(height, Buffer.from(gensig, 'hex')),
  calculateDeadlines: (calculateDeadlineData) => {
    calculateDeadlineData.forEach(data => data.genSig = Buffer.from(data.genSig, 'hex'));
    if (calculateDeadlineData.length === 0) {
      return [];
    }
    if (calculateDeadlineData.length === 1) {
      const dl = deadlineMath.calculate_deadline(
        calculateDeadlineData[0].accountId,
        calculateDeadlineData[0].nonce,
        calculateDeadlineData[0].scoopNr,
        calculateDeadlineData[0].baseTarget,
        calculateDeadlineData[0].genSig
      );
      calculateDeadlineData[0].deadline = parseInt(dl.toString(), 10);

      return calculateDeadlineData;
    }
    let populateTo = 4;
    if (calculateDeadlineData.length > 4) {
      populateTo = 8;
    }
    const calculateDeadlineDataOriginal = [...calculateDeadlineData];
    while (calculateDeadlineData.length < populateTo) {
      calculateDeadlineData.push(calculateDeadlineData[0]);
    }
    const func = populateTo === 4 ? deadlineMath.calculate_deadlines_sse4 : deadlineMath.calculate_deadlines_avx2;
    const dls = func(...calculateDeadlineData.map(data => [
      data.accountId,
      data.nonce,
      data.scoopNr,
      data.baseTarget,
      data.genSig,
    ]));
    const dlNum = dls.map(dl => parseInt(dl.toString(), 10));
    calculateDeadlineDataOriginal.forEach((data, index) => data.deadline = dlNum[index]);

    return calculateDeadlineDataOriginal;
  },
};
