/**
 * @file MultibandCompressor4.cpp
 * @brief MultibandCompressor4 实现
 *
 * 实现多频段压缩器：翘曲滤波器分频、逐频段前瞻增益、动态交叉频率。
 */

#include "utils/dsp196/MultibandCompressor4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

MultibandCompressor4::MultibandCompressor4(QObject *parent) : QObject(parent)
{
    setNumBands(4);
    setSampleRate(44100);
}

MultibandCompressor4::~MultibandCompressor4() = default;

/* ---- Configuration ---- */

void MultibandCompressor4::setSampleRate(int sr)
{
    m_sampleRate = qMax(8000, sr);
    m_stats.sampleRate = m_sampleRate;
    updateWarpCoeffs();
}

void MultibandCompressor4::setNumBands(int bands)
{
    m_numBands = qBound(2, bands, 8);
    m_params.resize(m_numBands);
    m_envLevel.resize(m_numBands, -60.0);
    m_gainReduction.resize(m_numBands, 0.0);
    m_stats.numBands = m_numBands;

    // Default crossover frequencies (log-spaced)
    m_crossFreqs.resize(m_numBands - 1);
    for (int i = 0; i < m_numBands - 1; ++i)
        m_crossFreqs[i] = 80.0 * qPow(1000.0 / 80.0,
            static_cast<double>(i + 1) / m_numBands);

    m_lookaheadBuf.resize(m_numBands);
    m_lookaheadPos.resize(m_numBands, 0);
    for (int b = 0; b < m_numBands; ++b)
        m_lookaheadBuf[b].resize(m_lookahead, 0.0);

    m_warpStates.resize(m_numBands - 1);
    for (auto& ws : m_warpStates)
        ws = QVector<WarpState>(2); // 2 allpass stages per crossover
}

void MultibandCompressor4::setBandParams(int band, const BandParams& params)
{
    if (band >= 0 && band < m_params.size())
        m_params[band] = params;
}

void MultibandCompressor4::setCrossFreq(int index, double freq)
{
    if (index >= 0 && index < m_crossFreqs.size()) {
        m_crossFreqs[index] = qBound(20.0, freq, m_sampleRate / 2.0 - 100.0);
        updateWarpCoeffs();
    }
}

void MultibandCompressor4::setLookahead(int samples)
{
    m_lookahead = qBound(0, samples, 2048);
    for (int b = 0; b < m_numBands; ++b) {
        m_lookaheadBuf[b].resize(m_lookahead, 0.0);
        m_lookaheadPos[b] = 0;
    }
}

void MultibandCompressor4::setWarpFactor(double lambda)
{
    m_warpFactor = qBound(-0.99, lambda, 0.99);
    updateWarpCoeffs();
}

/* ---- Update warped coefficients ---- */

void MultibandCompressor4::updateWarpCoeffs()
{
    // Compute 1st-order allpass coefficient for each crossover frequency
    // a = (1 - tan(pi*fc/fs)) / (1 + tan(pi*fc/fs))
    m_apCoeff.resize(m_crossFreqs.size());
    for (int i = 0; i < m_crossFreqs.size(); ++i) {
        double omega = M_PI * m_crossFreqs[i] / m_sampleRate;
        double t = qTan(omega);
        m_apCoeff[i] = (1.0 - t) / (1.0 + t);
    }
}

/* ---- Split bands via warped crossover ---- */

QVector<QVector<double>> MultibandCompressor4::splitBands(const QVector<double>& input)
{
    int n = input.size();
    QVector<QVector<double>> bands(m_numBands);

    // Apply warped crossover filters cascaded
    QVector<QVector<double>> cascade(m_numBands);
    QVector<double> lowPass(n), highPass(n);

    QVector<double> current = input;
    for (int i = 0; i < m_numBands - 1; ++i) {
        // First-order allpass-based crossover with warping
        double a = (i < m_apCoeff.size()) ? m_apCoeff[i] : 0.5;
        // Apply warping factor
        a = (a + m_warpFactor) / (1.0 + m_warpFactor * a);

        double xz = 0.0, yz = 0.0;
        for (int s = 0; s < n; ++s) {
            double x = current[s];
            double ap = a * (x - yz) + xz;
            xz = x;
            yz = ap;
            lowPass[s] = 0.5 * (x + ap);
            highPass[s] = 0.5 * (x - ap);
        }
        bands[i] = lowPass;
        current = highPass;
    }
    bands[m_numBands - 1] = current;
    return bands;
}

/* ---- Compute gain for one band ---- */

double MultibandCompressor4::computeGain(int band, double level) const
{
    if (band < 0 || band >= m_params.size()) return 0.0;
    const auto& p = m_params[band];

    double t = p.threshold;
    double r = p.ratio;
    double w = p.knee / 2.0;
    double gr = 0.0;

    if (level < t - w) {
        gr = 0.0;  // below threshold - no compression
    } else if (level > t + w) {
        gr = t + (level - t) / r - level;  // full compression
    } else {
        // Soft knee interpolation
        double x = level - t + w;
        gr = (x * x) / (4.0 * w) * (1.0 / r - 1.0);
    }

    return gr + p.makeupGain;
}

/* ---- Envelope follower ---- */

double MultibandCompressor4::followEnvelope(int band, double input)
{
    if (band < 0 || band >= m_params.size()) return 0.0;
    const auto& p = m_params[band];

    double samplePeriod = 1000.0 / m_sampleRate;
    double attackCoeff = qExp(-samplePeriod / qMax(p.attack, 0.01));
    double releaseCoeff = qExp(-samplePeriod / qMax(p.release, 0.01));

    double level = 20.0 * qLog10(qMax(qAbs(input), 1e-10)) / qLn(10);
    // Simplified: direct dB level
    if (level > m_envLevel[band])
        m_envLevel[band] = attackCoeff * m_envLevel[band] + (1.0 - attackCoeff) * level;
    else
        m_envLevel[band] = releaseCoeff * m_envLevel[band] + (1.0 - releaseCoeff) * level;

    return m_envLevel[band];
}

/* ---- Process ---- */

QVector<double> MultibandCompressor4::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    if (n == 0) return input;

    // Split into frequency bands
    auto bands = splitBands(input);
    QVector<double> output(n, 0.0);
    double peakGR = 0.0;

    for (int b = 0; b < m_numBands; ++b) {
        for (int s = 0; s < n; ++s) {
            // Envelope follower
            double level = followEnvelope(b, bands[b][s]);

            // Compute gain
            double gainDB = computeGain(b, level);
            m_gainReduction[b] = gainDB;
            peakGR = qMin(peakGR, gainDB);

            // Convert dB to linear gain
            double gainLin = qPow(10.0, gainDB / 20.0);

            // Lookahead: write to buffer, read delayed
            if (m_lookahead > 0) {
                int readPos = (m_lookaheadPos[b] + 1) % m_lookahead;
                double delayed = m_lookaheadBuf[b][readPos];
                m_lookaheadBuf[b][m_lookaheadPos[b]] = bands[b][s] * gainLin;
                m_lookaheadPos[b] = readPos;
                output[s] += delayed;
            } else {
                output[s] += bands[b][s] * gainLin;
            }
        }
    }

    m_stats.totalFrames += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(m_stats.totalFrames, 1ULL);

    emit frameProcessed(m_numBands, peakGR, timer.elapsed());
    return output;
}

/* ---- Process stereo ---- */

QVector<double> MultibandCompressor4::processStereo(const QVector<double>& input)
{
    // Interleaved stereo: L R L R ...
    int n = input.size();
    QVector<double> left(n / 2), right(n / 2);
    for (int i = 0; i < n / 2; ++i) {
        left[i] = input[2 * i];
        right[i] = input[2 * i + 1];
    }
    auto lOut = process(left);
    // Reset envelope to link channels
    auto rOut = process(right);

    QVector<double> result(n);
    for (int i = 0; i < n / 2; ++i) {
        result[2 * i] = lOut[i];
        result[2 * i + 1] = rOut[i];
    }
    return result;
}

/* ---- Accessors ---- */

QVector<double> MultibandCompressor4::bandLevels() const { return m_envLevel; }
QVector<double> MultibandCompressor4::gainReduction() const { return m_gainReduction; }

/* ---- Reset ---- */

void MultibandCompressor4::resetStatistics()
{
    m_stats = Stats{};
    m_stats.sampleRate = m_sampleRate;
    m_stats.numBands = m_numBands;
    m_timeSum = 0.0;
    m_envLevel.fill(-60.0);
    m_gainReduction.fill(0.0);
    for (auto& buf : m_lookaheadBuf) buf.fill(0.0);
    m_lookaheadPos.fill(0);
}
