/**
 * @file Compressor3.cpp
 * @brief Compressor3 实现
 *
 * 实现多频段立体声压缩器：中/侧编码、自适应交叉滤波、独立频段压缩。
 */

#include "utils/dsp194/Compressor3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

Compressor3::Compressor3(QObject *parent) : QObject(parent)
{
    setCrossoverFreqs({120.0, 1000.0, 4000.0});
}

Compressor3::~Compressor3() = default;

/* ---- Configuration ---- */

void Compressor3::setNumBands(int b) { m_numBands = qMax(1, b); }
void Compressor3::setSampleRate(double sr) { m_sampleRate = qMax(8000.0, sr); designCrossovers(); }
void Compressor3::setThreshold(double db) { m_threshold = db; }
void Compressor3::setRatio(double r) { m_ratio = qMax(1.0, r); }
void Compressor3::setAttack(double ms) { m_attack = qMax(0.1, ms); }
void Compressor3::setRelease(double ms) { m_release = qMax(1.0, ms); }
void Compressor3::setKnee(double db) { m_knee = qMax(0.0, db); }

void Compressor3::setCrossoverFreqs(const QVector<double>& freqs)
{
    m_crossoverFreqs = freqs;
    m_numBands = freqs.size() + 1;
    m_bandGR.resize(m_numBands);
    m_envMid.resize(m_numBands);
    m_envSide.resize(m_numBands);
    m_lpCoeffs.resize(m_numBands);
    m_hpCoeffs.resize(m_numBands);
    m_lpState.resize(m_numBands, QVector<double>(4, 0.0));
    m_hpState.resize(m_numBands, QVector<double>(4, 0.0));
    m_bandGR.fill(0.0);
    m_envMid.fill(0.0);
    m_envSide.fill(0.0);
    designCrossovers();
}

/* ---- Design Linkwitz-Riley crossovers ---- */

void Compressor3::designCrossovers()
{
    // Each crossover: 2nd-order Butterworth LP/HP (cascade for LR4)
    // coeff = [b0, b1, b2, a1, a2]
    for (int b = 0; b < m_crossoverFreqs.size(); ++b) {
        double fc = m_crossoverFreqs[b];
        double omega = 2.0 * M_PI * fc / m_sampleRate;
        double sinO = qSin(omega);
        double cosO = qCos(omega);
        // Butterworth Q = 0.7071
        double alpha = sinO / (2.0 * 0.7071);

        // LP coefficients
        double lpB0 = (1.0 - cosO) / 2.0;
        double lpB1 = 1.0 - cosO;
        double lpB2 = lpB0;
        double a0 = 1.0 + alpha;
        double a1 = -2.0 * cosO;
        double a2 = 1.0 - alpha;
        m_lpCoeffs[b] = {lpB0 / a0, lpB1 / a0, lpB2 / a0, a1 / a0, a2 / a0};

        // HP coefficients
        double hpB0 = (1.0 + cosO) / 2.0;
        double hpB1 = -(1.0 + cosO);
        double hpB2 = hpB0;
        m_hpCoeffs[b] = {hpB0 / a0, hpB1 / a0, hpB2 / a0, a1 / a0, a2 / a0};
    }
}

/* ---- Biquad filter ---- */

double Compressor3::biquad(double x, const QVector<double>& c, QVector<double>& s)
{
    double y = c[0] * x + c[1] * s[0] + c[2] * s[1] - c[3] * s[2] - c[4] * s[3];
    s[1] = s[0]; s[0] = x;
    s[3] = s[2]; s[2] = y;
    return y;
}

/* ---- Soft-knee compression gain ---- */

double Compressor3::compressGain(double levelDB) const
{
    double t = m_threshold;
    double r = m_ratio;
    double k = m_knee / 2.0;

    if (m_knee <= 0.0) {
        // Hard knee
        if (levelDB <= t) return 0.0;
        return (t - levelDB) * (1.0 - 1.0 / r);
    }
    // Soft knee
    double delta = levelDB - t;
    if (delta < -k) return 0.0;
    if (delta > k) return (t - levelDB) * (1.0 - 1.0 / r);
    // Quadratic interpolation in knee region
    double x = delta + k;
    return -x * x / (4.0 * k) * (1.0 - 1.0 / r);
}

/* ---- M/S encode ---- */

QPair<QVector<double>, QVector<double>> Compressor3::encodeMS(
    const QVector<double>& left, const QVector<double>& right) const
{
    int n = qMin(left.size(), right.size());
    QVector<double> mid(n), side(n);
    for (int i = 0; i < n; ++i) {
        mid[i] = (left[i] + right[i]) * 0.5;
        side[i] = (left[i] - right[i]) * 0.5;
    }
    return {mid, side};
}

/* ---- M/S decode ---- */

QPair<QVector<double>, QVector<double>> Compressor3::decodeMS(
    const QVector<double>& mid, const QVector<double>& side) const
{
    int n = qMin(mid.size(), side.size());
    QVector<double> left(n), right(n);
    for (int i = 0; i < n; ++i) {
        left[i] = mid[i] + side[i];
        right[i] = mid[i] - side[i];
    }
    return {left, right};
}

/* ---- Process stereo block ---- */

QVector<double> Compressor3::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size() / 2;  // stereo pairs
    if (n == 0) return input;

    // De-interleave
    QVector<double> L(n), R(n);
    for (int i = 0; i < n; ++i) { L[i] = input[2 * i]; R[i] = input[2 * i + 1]; }

    // Encode to M/S
    auto ms = encodeMS(L, R);
    QVector<double>& mid = ms.first;
    QVector<double>& side = ms.second;

    // Envelope parameters
    double attCoeff = qExp(-1.0 / (m_attack * m_sampleRate / 1000.0));
    double relCoeff = qExp(-1.0 / (m_release * m_sampleRate / 1000.0));

    // Process each band
    QVector<double> outMid(n, 0.0), outSide(n, 0.0);
    double totalGR = 0.0;

    for (int b = 0; b < m_numBands; ++b) {
        QVector<double> bandMid(n), bandSide(n);

        // Apply crossover filtering (simplified: use cascade of HP/LP)
        if (b < m_crossoverFreqs.size()) {
            for (int i = 0; i < n; ++i) {
                bandMid[i] = biquad(mid[i], m_lpCoeffs[b], m_lpState[b]);
                bandSide[i] = biquad(side[i], m_lpCoeffs[b], m_lpState[b]);
            }
        } else {
            bandMid = mid;
            bandSide = side;
        }

        // Compress each sample with envelope follower
        double bandGRSum = 0.0;
        for (int i = 0; i < n; ++i) {
            // Envelope detection (peak)
            double absM = qAbs(bandMid[i]);
            double absS = qAbs(bandSide[i]);
            double peak = qMax(absM, absS);

            double coeff = (peak > m_envMid[b]) ? attCoeff : relCoeff;
            m_envMid[b] = coeff * m_envMid[b] + (1.0 - coeff) * peak;

            double levelDB = 20.0 * qLn(qMax(m_envMid[b], 1e-10)) / M_LN10;
            double gainDB = compressGain(levelDB);
            double gain = qPow(10.0, gainDB / 20.0);

            bandMid[i] *= gain;
            bandSide[i] *= gain;
            bandGRSum += gainDB;

            outMid[i] += bandMid[i];
            outSide[i] += bandSide[i];
        }
        m_bandGR[b] = (n > 0) ? bandGRSum / n : 0.0;
        totalGR += m_bandGR[b];
    }

    // Decode M/S back to L/R
    auto lr = decodeMS(outMid, outSide);

    // Re-interleave
    QVector<double> output(input.size());
    double inLvl = 0.0, outLvl = 0.0;
    for (int i = 0; i < n; ++i) {
        output[2 * i] = lr.first[i];
        output[2 * i + 1] = lr.second[i];
        inLvl += qAbs(L[i]) + qAbs(R[i]);
        outLvl += qAbs(lr.first[i]) + qAbs(lr.second[i]);
    }

    m_stats.totalBlocks++;
    m_stats.numBands = m_numBands;
    m_stats.avgInputLevel = (inLvl / (2 * n));
    m_stats.avgOutputLevel = (outLvl / (2 * n));
    m_stats.avgGainReduction = totalGR / m_numBands;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocks;

    emit blockProcessed(m_numBands, totalGR / m_numBands, timer.elapsed());
    return output;
}

/* ---- Gain reduction ---- */

QVector<double> Compressor3::gainReduction() const { return m_bandGR; }

/* ---- Reset ---- */

void Compressor3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_bandGR.fill(0.0);
    m_envMid.fill(0.0);
    m_envSide.fill(0.0);
    for (auto& s : m_lpState) s.fill(0.0);
    for (auto& s : m_hpState) s.fill(0.0);
}
