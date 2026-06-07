/**
 * @file Deesser4.cpp
 * @brief Deesser4 实现
 *
 * 实现去齿音处理器：MFCC特征提取、齿音评分、自适应阈值增益抑制。
 */

#include "utils/dsp204/Deesser4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>

/* ---- Construction / Destruction ---- */

Deesser4::Deesser4(QObject *parent) : QObject(parent)
{
    buildMelFilterBank();
    buildDCT();
    buildWindow();
}

Deesser4::~Deesser4() = default;

/* ---- Configuration ---- */

void Deesser4::setSampleRate(double rate) { m_sampleRate = qMax(8000.0, rate); buildMelFilterBank(); }
void Deesser4::setFrameSize(int size) { m_frameSize = qMax(256, size); buildMelFilterBank(); buildWindow(); }
void Deesser4::setSibilanceThreshold(double thresh) { m_threshold = qBound(0.0, thresh, 1.0); }
void Deesser4::setReductionGain(double gainDb) { m_reductionGain = gainDb; }
void Deesser4::setAdaptationRate(double rate) { m_adaptRate = qBound(0.001, rate, 0.1); }

/* ---- Mel conversion ---- */

double Deesser4::hzToMel(double hz) { return 1127.0 * qLn(1.0 + hz / 700.0); }
double Deesser4::melToHz(double mel) { return 700.0 * (qExp(mel / 1127.0) - 1.0); }

/* ---- Build Mel filter bank ---- */

void Deesser4::buildMelFilterBank()
{
    int numBands = 26;
    int fftSize = m_frameSize;
    double maxMel = hzToMel(m_sampleRate / 2.0);
    double melStep = maxMel / (numBands + 1);

    m_melFilterBank = QVector<QVector<double>>(numBands, QVector<double>(fftSize / 2 + 1, 0.0));

    QVector<double> melCenters(numBands + 2);
    for (int i = 0; i < numBands + 2; ++i) melCenters[i] = i * melStep;

    for (int b = 0; b < numBands; ++b) {
        double fLeft = melToHz(melCenters[b]);
        double fCenter = melToHz(melCenters[b + 1]);
        double fRight = melToHz(melCenters[b + 2]);

        int kLeft = qRound(fLeft * fftSize / m_sampleRate);
        int kCenter = qRound(fCenter * fftSize / m_sampleRate);
        int kRight = qRound(fRight * fftSize / m_sampleRate);

        for (int k = qMax(0, kLeft); k <= qMin(fftSize / 2, kRight); ++k) {
            if (k <= kCenter && kCenter > kLeft)
                m_melFilterBank[b][k] = static_cast<double>(k - kLeft) / (kCenter - kLeft);
            else if (k > kCenter && kRight > kCenter)
                m_melFilterBank[b][k] = static_cast<double>(kRight - k) / (kRight - kCenter);
        }
    }
}

/* ---- Build DCT coefficients ---- */

void Deesser4::buildDCT()
{
    int numBands = 26;
    int numCoeffs = 13;
    m_dctCoeffs.resize(numCoeffs * numBands);

    for (int n = 0; n < numCoeffs; ++n)
        for (int k = 0; k < numBands; ++k)
            m_dctCoeffs[n * numBands + k] = qCos(M_PI * n * (2 * k + 1) / (2.0 * numBands));
}

/* ---- Build Hann window ---- */

void Deesser4::buildWindow()
{
    m_hannWindow.resize(m_frameSize);
    for (int i = 0; i < m_frameSize; ++i)
        m_hannWindow[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * i / (m_frameSize - 1)));
}

/* ---- Apply window ---- */

void Deesser4::applyWindow(QVector<double>& frame) const
{
    for (int i = 0; i < qMin(frame.size(), m_hannWindow.size()); ++i)
        frame[i] *= m_hannWindow[i];
}

/* ---- Power spectrum ---- */

QVector<double> Deesser4::powerSpectrum(const QVector<double>& windowed) const
{
    int n = windowed.size();
    QVector<double> mag(n / 2 + 1);
    // Simplified DFT magnitude for sibilance frequency range (4-10 kHz)
    for (int k = 0; k <= n / 2; ++k) {
        double re = 0.0, im = 0.0;
        for (int i = 0; i < n; ++i) {
            double angle = 2.0 * M_PI * k * i / n;
            re += windowed[i] * qCos(angle);
            im -= windowed[i] * qSin(angle);
        }
        mag[k] = (re * re + im * im) / n;
    }
    return mag;
}

/* ---- Compute MFCC ---- */

QVector<double> Deesser4::computeMFCC(const QVector<double>& frame) const
{
    QVector<double> windowed = frame;
    const_cast<Deesser4*>(this)->applyWindow(windowed);
    QVector<double> ps = powerSpectrum(windowed);

    int numBands = 26;
    int numCoeffs = 13;
    QVector<double> melEnergies(numBands, 0.0);

    // Apply Mel filter bank
    for (int b = 0; b < numBands; ++b) {
        for (int k = 0; k < ps.size(); ++k)
            melEnergies[b] += m_melFilterBank[b][k] * ps[k];
        melEnergies[b] = qLn(qMax(melEnergies[b], 1e-10));
    }

    // Apply DCT
    QVector<double> mfcc(numCoeffs);
    for (int n = 0; n < numCoeffs; ++n) {
        double sum = 0.0;
        for (int k = 0; k < numBands; ++k)
            sum += m_dctCoeffs[n * numBands + k] * melEnergies[k];
        mfcc[n] = sum;
    }
    return mfcc;
}

/* ---- Sibilance score ---- */

double Deesser4::sibilanceScore(const QVector<double>& mfcc) const
{
    // Weight high-frequency cepstral coefficients (sibilance in 4-10kHz)
    // Higher MFCC indices capture finer spectral detail
    double score = 0.0;
    double totalWeight = 0.0;
    for (int i = 4; i < qMin(13, mfcc.size()); ++i) {
        double w = static_cast<double>(i) / 13.0;
        score += qAbs(mfcc[i]) * w;
        totalWeight += w;
    }
    if (totalWeight > 0) score /= totalWeight;

    // Normalize to 0..1 range with sigmoid
    score = 1.0 / (1.0 + qExp(-3.0 * (score - 2.0)));
    return qBound(0.0, score, 1.0);
}

/* ---- Apply reduction ---- */

void Deesser4::applyReduction(QVector<double>& spectrum, double score) const
{
    if (score <= m_threshold) return;

    double reductionFactor = score - m_threshold;
    double gainDb = m_reductionGain * reductionFactor;
    double gain = qPow(10.0, gainDb / 20.0);

    // Target sibilant frequency range: 4-10 kHz bins
    double binHz = m_sampleRate / m_frameSize;
    int lowBin = qMax(0, static_cast<int>(4000.0 / binHz));
    int highBin = qMin(spectrum.size() - 1, static_cast<int>(10000.0 / binHz));

    for (int k = lowBin; k <= highBin; ++k)
        spectrum[k] *= gain;
}

/* ---- Adapt threshold ---- */

void Deesser4::adaptThreshold(double currentScore)
{
    m_sibilanceSum += currentScore;
    m_frameCount++;
    double mean = m_sibilanceSum / m_frameCount;

    double diff = currentScore - mean;
    m_runningStd = qSqrt(m_runningStd * m_runningStd * (1.0 - m_adaptRate) + diff * diff * m_adaptRate);
    m_runningMean = m_runningMean * (1.0 - m_adaptRate) + currentScore * m_adaptRate;

    // Shift threshold toward running mean + 1.5 sigma
    double targetThresh = m_runningMean + 1.5 * m_runningStd;
    m_threshold = m_threshold * (1.0 - m_adaptRate) + targetThresh * m_adaptRate;
    m_threshold = qBound(0.3, m_threshold, 0.9);
}

/* ---- Process frame ---- */

QVector<double> Deesser4::process(const QVector<double>& frame)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> mfcc = computeMFCC(frame);
    double score = sibilanceScore(mfcc);

    // Compute power spectrum for reduction
    QVector<double> windowed = frame;
    applyWindow(windowed);
    QVector<double> ps = powerSpectrum(windowed);

    applyReduction(ps, score);
    adaptThreshold(score);

    m_stats.totalFrames++;
    m_stats.frameSize = frame.size();
    m_sibilanceSum += score;
    m_stats.avgSibilanceScore = m_sibilanceSum / m_stats.totalFrames;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFrames;

    emit frameProcessed(score, m_reductionGain * (score - m_threshold), timer.elapsed());
    return ps;
}

/* ---- Reset ---- */

void Deesser4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_runningMean = 0.0;
    m_runningStd = 0.0;
    m_sibilanceSum = 0.0;
    m_frameCount = 0;
}
