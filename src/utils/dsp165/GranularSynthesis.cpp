/**
 * @file GranularSynthesis.cpp
 * @brief GranularSynthesis 实现
 *
 * 实现颗粒合成引擎：粒子云生成、包络计算、重叠叠加合成。
 */

#include "utils/dsp165/GranularSynthesis.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

GranularSynthesis::GranularSynthesis(QObject* parent)
    : QObject(parent)
{
}

GranularSynthesis::~GranularSynthesis() = default;

void GranularSynthesis::setSampleRate(int rate) { m_sampleRate = qMax(8000, rate); }
void GranularSynthesis::setGrainDuration(double ms) { m_grainDuration = qMax(1.0, ms); }
void GranularSynthesis::setGrainDensity(double density) { m_grainDensity = qMax(1.0, density); }
void GranularSynthesis::setPositionScatter(double scatter) { m_positionScatter = qBound(0.0, scatter, 1.0); }
void GranularSynthesis::setEnvelope(Envelope env) { m_envelope = env; }

QVector<GranularSynthesis::Grain> GranularSynthesis::generateGrainCloud(
    int sourceLen, int outputLen) const
{
    QVector<Grain> grains;
    double outputDuration = static_cast<double>(outputLen) / m_sampleRate;
    int grainSamples = static_cast<int>(m_grainDuration * m_sampleRate / 1000.0);
    grainSamples = qMax(2, grainSamples);

    /* Number of grains based on density */
    int totalGrains = static_cast<int>(outputDuration * m_grainDensity);
    totalGrains = qMax(1, totalGrains);

    /* Seed a simple LCG for reproducible scatter */
    quint32 seed = 42;
    auto rand01 = [&seed]() -> double {
        seed = seed * 1103515245 + 12345;
        return (seed >> 16) / 65536.0;
    };

    for (int i = 0; i < totalGrains; ++i) {
        Grain g;
        g.startTime = rand01() * outputDuration;
        g.duration = grainSamples;

        /* Position with scatter */
        double centerPos = (static_cast<double>(i) / totalGrains) * sourceLen;
        double scatterRange = m_positionScatter * sourceLen * 0.5;
        g.position = centerPos + (rand01() - 0.5) * 2.0 * scatterRange;
        g.position = qBound(0.0, g.position, static_cast<double>(sourceLen - grainSamples));

        g.pitch = 1.0; /* No pitch shifting */
        g.amplitude = 0.8 + rand01() * 0.2;
        grains.append(g);
    }

    /* Sort by start time */
    std::sort(grains.begin(), grains.end(),
              [](const Grain& a, const Grain& b) { return a.startTime < b.startTime; });

    return grains;
}

QVector<double> GranularSynthesis::computeEnvelope(int grainLen) const
{
    QVector<double> env(grainLen);
    for (int i = 0; i < grainLen; ++i) {
        double t = static_cast<double>(i) / (grainLen - 1);
        switch (m_envelope) {
        case Hanning:
            env[i] = 0.5 * (1.0 - qCos(2.0 * M_PI * t));
            break;
        case Gaussian: {
            double sigma = 0.25;
            double x = (t - 0.5) / sigma;
            env[i] = qExp(-0.5 * x * x);
            break;
        }
        case Trapezoid: {
            double attack = 0.1, release = 0.1;
            if (t < attack)
                env[i] = t / attack;
            else if (t > 1.0 - release)
                env[i] = (1.0 - t) / release;
            else
                env[i] = 1.0;
            break;
        }
        }
    }
    return env;
}

void GranularSynthesis::extractGrain(const QVector<double>& source, const Grain& g,
                                     QVector<double>& out) const
{
    int grainLen = qMin(g.duration, static_cast<int>(source.size() - g.position));
    grainLen = qMax(0, grainLen);
    out.resize(grainLen);

    QVector<double> env = computeEnvelope(grainLen);

    int start = static_cast<int>(g.position);
    for (int i = 0; i < grainLen; ++i) {
        int srcIdx = start + i;
        if (srcIdx >= 0 && srcIdx < source.size()) {
            out[i] = source[srcIdx] * env[i] * g.amplitude;
        } else {
            out[i] = 0.0;
        }
    }
}

QVector<double> GranularSynthesis::synthesize(const QVector<double>& source, int outputLength)
{
    QElapsedTimer timer;
    timer.start();

    if (source.isEmpty() || outputLength <= 0) return QVector<double>();

    QVector<Grain> grains = generateGrainCloud(source.size(), outputLength);
    QVector<double> output(outputLength, 0.0);

    for (const Grain& g : grains) {
        QVector<double> grainData;
        extractGrain(source, g, grainData);

        int startSample = static_cast<int>(g.startTime * m_sampleRate);
        for (int i = 0; i < grainData.size(); ++i) {
            int outIdx = startSample + i;
            if (outIdx >= 0 && outIdx < outputLength) {
                output[outIdx] += grainData[i];
            }
        }
    }

    /* Normalize to prevent clipping */
    double maxVal = 0.0;
    for (double v : output) maxVal = qMax(maxVal, qAbs(v));
    if (maxVal > 1.0) {
        for (int i = 0; i < output.size(); ++i) output[i] /= maxVal;
    }

    m_stats.totalSyntheses++;
    m_stats.lastGrainCount = grains.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSyntheses > 0)
        ? m_timeSum / m_stats.totalSyntheses : 0.0;

    emit synthesisCompleted(grains.size());
    return output;
}

void GranularSynthesis::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
