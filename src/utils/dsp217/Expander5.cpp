/**
 * @file Expander5.cpp
 * @brief Expander5 实现
 *
 * 实现多频段扩展器：心理声学加权阈值、瞬态保护释放曲线、交叉滤波。
 */

#include "utils/dsp217/Expander5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

Expander5::Expander5(QObject *parent) : QObject(parent) {}
Expander5::~Expander5() = default;

/* ---- Configuration ---- */

void Expander5::setBands(const QVector<BandConfig>& bands)
{
    m_bands = bands;
    m_envelope.resize(bands.size(), -120.0);
    m_gainReduction.resize(bands.size(), 0.0);
    m_prevTransient.resize(bands.size(), 0.0);
    m_stats.numBands = bands.size();
}

void Expander5::setPsychoacousticMode(int mode)
{
    m_psychoMode = qBound(0, mode, 2);
}

/* ---- Psychoacoustic weighting ---- */

double Expander5::psychoWeight(double freq) const
{
    if (m_psychoMode == 0) return 0.0;  // No weighting

    // Simplified A-weighting approximation (dB offset)
    double f2 = freq * freq;
    double aWeight = 1.99926 * qLog(freq) / qLn(10)
        - 0.0000002993 * f2 * f2
        - 0.0014029 * f2 + 1.0;
    if (m_psychoMode == 1) return aWeight;

    // Simplified ISO 226 equal-loudness contour
    double af = 0.00447 * qPow(freq, 0.3);
    double lnF = qLn(qMax(freq, 20.0));
    double threshold = 3.64 * qPow(freq / 1000.0, -0.8)
        - 6.5 * qExp(-0.6 * qPow((lnF - qLn(6000.0)) / qLn(1000.0), 2))
        + 0.001 * qPow(freq / 1000.0, 4);
    return threshold;
}

/* ---- Transient detection ---- */

bool Expander5::isTransient(double input, int bandIdx) const
{
    // Transient if current input >> previous with fast onset
    double prev = (bandIdx < m_prevTransient.size()) ? m_prevTransient[bandIdx] : 0.0;
    return qAbs(input) > qAbs(prev) * 2.5;
}

/* ---- Envelope follower ---- */

double Expander5::envelopeFollow(double input, double attack,
                                   double release, int bandIdx)
{
    double coeff;
    bool transient = isTransient(input, bandIdx);
    if (transient) {
        coeff = 1.0 - qExp(-1.0 / (attack * 0.001 * 44100.0));
    } else {
        // Slower release to preserve transients
        double effectiveRelease = release * (transient ? 0.5 : 1.0);
        coeff = 1.0 - qExp(-1.0 / (effectiveRelease * 0.001 * 44100.0));
    }

    double absIn = qAbs(input);
    double& env = m_envelope[bandIdx];
    if (absIn > env) env += coeff * (absIn - env);
    else env -= coeff * (env - absIn);
    return env;
}

/* ---- Compute gain ---- */

double Expander5::computeGain(double inputDb, double threshold,
                                double ratio, int bandIdx)
{
    if (inputDb > threshold) return 0.0;

    double depth = threshold - inputDb;
    double expansion = depth * (ratio - 1.0) / ratio;
    return -expansion;
}

/* ---- Crossover filter (simplified Linkwitz-Riley) ---- */

void Expander5::crossoverFilter(const QVector<double>& input,
                                  QVector<QVector<double>>& bandOutputs)
{
    int n = input.size();
    int nb = m_bands.size();
    bandOutputs.resize(nb);

    for (int b = 0; b < nb; ++b) {
        bandOutputs[b].resize(n);
        double loF = m_bands[b].lowFreq;
        double hiF = m_bands[b].highFreq;
        // Simple first-order bandpass approximation
        double loC = qExp(-2.0 * M_PI * loF / 44100.0);
        double hiC = qExp(-2.0 * M_PI * hiF / 44100.0);
        double prev = 0.0;
        for (int i = 0; i < n; ++i) {
            double lp = prev + (1.0 - loC) * input[i];
            prev = lp;
            double bp = lp * hiC;
            bandOutputs[b][i] = input[i] * (1.0 - loC) * hiC;
        }
    }
}

/* ---- Process ---- */

QVector<double> Expander5::process(const QVector<double>& input)
{
    QElapsedTimer timer;
    timer.start();

    int n = input.size();
    int nb = m_bands.size();
    if (nb == 0) return input;

    QVector<QVector<double>> bandOutputs;
    crossoverFilter(input, bandOutputs);

    QVector<double> output(n, 0.0);
    double totalGR = 0.0;

    for (int b = 0; b < nb; ++b) {
        double psychoOff = psychoWeight((m_bands[b].lowFreq + m_bands[b].highFreq) / 2.0);
        double threshold = m_bands[b].threshold + psychoOff;
        double ratio = m_bands[b].ratio;
        double attack = m_bands[b].attack;
        double release = m_bands[b].release;
        double makeup = qPow(10.0, m_bands[b].makeupGain / 20.0);

        double bandGR = 0.0;
        for (int i = 0; i < n; ++i) {
            double env = envelopeFollow(bandOutputs[b][i], attack, release, b);
            double envDb = 20.0 * qLog10(qMax(env, 1e-10));
            double gr = computeGain(envDb, threshold, ratio, b);
            double gain = qPow(10.0, gr / 20.0);
            output[i] += bandOutputs[b][i] * gain * makeup;
            bandGR += gr;
        }
        m_gainReduction[b] = (n > 0) ? bandGR / n : 0.0;
        totalGR += qAbs(m_gainReduction[b]);

        // Update transient detector
        if (n > 0) m_prevTransient[b] = bandOutputs[b][n - 1];
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    double avgGR = (nb > 0) ? totalGR / nb : 0.0;
    emit processingCompleted(n, avgGR, timer.elapsed());
    return output;
}

/* ---- Accessors ---- */

QVector<double> Expander5::gainReduction() const { return m_gainReduction; }

/* ---- Reset ---- */

void Expander5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_envelope.clear();
    m_gainReduction.clear();
    m_prevTransient.clear();
}
