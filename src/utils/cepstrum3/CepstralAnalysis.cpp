#include "utils/cepstrum3/CepstralAnalysis.h"
#include <QElapsedTimer>
#include <QtMath>

CepstralAnalysis::CepstralAnalysis(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QVector<double> CepstralAnalysis::compute(const QVector<double>& signal) {
    QElapsedTimer timer; timer.start();
    int N = signal.size();
    QVector<double> cepstrum(N, 0.0);
    if (N < 4) return cepstrum;

    /* 功率谱(简化DFT) */
    QVector<double> power(N / 2);
    for (int k = 0; k < N / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = 2.0 * M_PI * k * n / N;
            re += signal[n] * qCos(angle);
            im -= signal[n] * qSin(angle);
        }
        power[k] = re * re + im * im;
    }

    /* 对数功率谱 */
    QVector<double> logPow(N / 2);
    for (int i = 0; i < N / 2; ++i)
        logPow[i] = qLn(qMax(power[i], 1e-30));

    /* IDFT得到倒谱 */
    for (int q = 0; q < qMin(N / 2, N); ++q) {
        double sum = 0.0;
        for (int k = 0; k < N / 2; ++k)
            sum += logPow[k] * qCos(2.0 * M_PI * k * q / (N / 2));
        cepstrum[q] = sum * 2.0 / (N / 2);
    }

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalAnalyses;
    m_stats.avgProcessingTimeMs = m_timeSum / static_cast<double>(m_stats.totalAnalyses);
    emit analysisCompleted(0.0);
    return cepstrum;
}

double CepstralAnalysis::getPitch(const QVector<double>& cepstrum,
                                   double sampleRate) const {
    int N = cepstrum.size();
    int minQ = static_cast<int>(sampleRate / 500.0);  // 500Hz上限
    int maxQ = static_cast<int>(sampleRate / 50.0);    // 50Hz下限
    minQ = qMax(1, minQ);
    maxQ = qMin(N - 1, maxQ);

    double bestVal = -1e18;
    int bestQ = 0;
    for (int q = minQ; q <= maxQ; ++q) {
        if (cepstrum[q] > bestVal) { bestVal = cepstrum[q]; bestQ = q; }
    }
    return (bestQ > 0) ? sampleRate / bestQ : 0.0;
}

QVector<double> CepstralAnalysis::getFormants(
    const QVector<double>& cepstrum, int nFormants) const {
    /* 在低倒频区找峰值 */
    QVector<double> formants;
    int maxQ = qMin(50, cepstrum.size() - 1);
    for (int i = 2; i < maxQ && formants.size() < static_cast<size_t>(nFormants); ++i) {
        if (cepstrum[i] > cepstrum[i-1] && cepstrum[i] > cepstrum[i+1] && cepstrum[i] > 0)
            formants.append(cepstrum[i]);
    }
    while (static_cast<int>(formants.size()) < nFormants)
        formants.append(0.0);
    return formants;
}

void CepstralAnalysis::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
