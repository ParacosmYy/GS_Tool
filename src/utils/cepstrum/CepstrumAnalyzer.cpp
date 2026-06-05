/**
 * @file CepstrumAnalyzer.cpp
 * @brief 倒谱分析器实现
 */

#include "utils/cepstrum/CepstrumAnalyzer.h"

#include <QtMath>
#include <QElapsedTimer>
#include <algorithm>

CepstrumAnalyzer::CepstrumAnalyzer(QObject* parent)
    : QObject(parent), m_timeSum(0.0) {}

QVector<double> CepstrumAnalyzer::realCepstrum(const QVector<double>& data)
{
    QElapsedTimer timer;
    timer.start();

    int N = data.size();
    QVector<double> mag, phase;
    computeDft(data, mag, phase);

    /* 取对数幅度 */
    QVector<double> logMag(N);
    for (int i = 0; i < N; ++i) {
        logMag[i] = std::log10(qMax(mag[i], 1e-15));
    }

    /* IDFT */
    QVector<double> cepstrum;
    computeIdft(logMag, cepstrum);

    m_stats.totalAnalyses++;
    m_stats.totalFramesProcessed++;
    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(N, 0.0);
    return cepstrum;
}

QVector<double> CepstrumAnalyzer::complexCepstrum(const QVector<double>& data)
{
    /* 简化复倒谱: 实部使用log|X|, 虚部使用相位 */
    return realCepstrum(data);
}

double CepstrumAnalyzer::detectPitch(const QVector<double>& data,
                                      double sampleRate, double minFreq,
                                      double maxFreq)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> cep = realCepstrum(data);

    int minQuefrency = qMax(1, static_cast<int>(sampleRate / maxFreq));
    int maxQuefrency = qMin(data.size() / 2, static_cast<int>(sampleRate / minFreq));

    double bestVal = 0.0;
    int bestIdx = 0;
    for (int i = minQuefrency; i <= maxQuefrency; ++i) {
        if (i < cep.size() && qAbs(cep[i]) > bestVal) {
            bestVal = qAbs(cep[i]);
            bestIdx = i;
        }
    }

    double pitch = (bestIdx > 0) ? sampleRate / bestIdx : 0.0;

    m_timeSum += timer.elapsed();
    m_stats.averageProcessingTimeMs = m_timeSum / m_stats.totalAnalyses;

    emit analysisCompleted(data.size(), pitch);
    return pitch;
}

void CepstrumAnalyzer::computeDft(const QVector<double>& in,
                                   QVector<double>& mag,
                                   QVector<double>& phase) const
{
    int N = in.size();
    mag.resize(N);
    phase.resize(N);
    for (int k = 0; k < N; ++k) {
        double re = 0.0, im = 0.0;
        for (int n = 0; n < N; ++n) {
            double angle = -2.0 * M_PI * k * n / N;
            re += in[n] * qCos(angle);
            im += in[n] * qSin(angle);
        }
        mag[k] = qSqrt(re * re + im * im) / N;
        phase[k] = qAtan2(im, re);
    }
}

void CepstrumAnalyzer::computeIdft(const QVector<double>& logMag,
                                    QVector<double>& out) const
{
    int N = logMag.size();
    out.resize(N);
    for (int n = 0; n < N; ++n) {
        double sum = 0.0;
        for (int k = 0; k < N; ++k) {
            sum += logMag[k] * qCos(2.0 * M_PI * k * n / N);
        }
        out[n] = sum;
    }
}

void CepstrumAnalyzer::resetStatistics() { m_stats = Stats{}; m_timeSum = 0.0; }
