/**
 * @file SignalGenerator.cpp
 * @brief 信号发生器实现
 */

#include "utils/signalgen/SignalGenerator.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>

SignalGenerator::SignalGenerator(QObject* parent)
    : QObject(parent)
    , m_waveformType(WaveformType::Sine)
    , m_seed(0)
    , m_seedSet(false)
    , m_timeSum(0.0)
{}

void SignalGenerator::setWaveformType(WaveformType type)
{
    m_waveformType = type;
}

void SignalGenerator::setWaveformParams(const WaveformParams& params)
{
    m_params = params;
}

void SignalGenerator::setModulationParams(const ModulationParams& params)
{
    m_modParams = params;
}

void SignalGenerator::setSeed(quint32 seed)
{
    m_seed = seed;
    m_seedSet = true;
}

double SignalGenerator::computeBase(double t) const
{
    double phase = m_params.phase + 2.0 * M_PI * m_params.frequency * t;

    switch (m_waveformType) {
    case WaveformType::Sine:
        return qSin(phase);

    case WaveformType::Square:
        return (qSin(phase) >= 0.0) ? 1.0 : -1.0;

    case WaveformType::Triangle: {
        /* 三角波: 锯齿的绝对值变换 */
        double p = phase / (2.0 * M_PI);
        p = p - qFloor(p); // 归一化到 [0, 1)
        return 4.0 * qAbs(p - 0.5) - 1.0;
    }

    case WaveformType::Sawtooth: {
        double p = phase / (2.0 * M_PI);
        p = p - qFloor(p);
        return 2.0 * p - 1.0;
    }

    case WaveformType::Noise: {
        /* 使用静态引擎避免重复构造 */
        static thread_local std::mt19937 gen(std::random_device{}());
        static thread_local std::uniform_real_distribution<double> dist(-1.0, 1.0);
        return dist(gen);
    }
    }
    return 0.0;
}

double SignalGenerator::applyModulation(double value, double t) const
{
    switch (m_modParams.type) {
    case ModulationType::None:
        return value;

    case ModulationType::AM: {
        /* 幅度调制: y(t) = [1 + m*cos(2π*fm*t)] * carrier */
        double modulator = 1.0 + m_modParams.modDepth *
            qCos(2.0 * M_PI * m_modParams.modFrequency * t);
        return value * modulator;
    }

    case ModulationType::FM: {
        /* FM在sampleAt中通过频率偏移实现，此处不重复处理 */
        return value;
    }
    }
    return value;
}

double SignalGenerator::sampleAt(double t) const
{
    double baseValue = 0.0;

    if (m_waveformType == WaveformType::Noise) {
        /* 噪声不参与调制，直接返回 */
        baseValue = computeBase(t);
    } else if (m_modParams.type == ModulationType::FM) {
        /* FM调制: 瞬时频率 f(t) = fc + β*fm*cos(2π*fm*t)
           积分后相位: φ(t) = 2π*fc*t + (β/2π)*sin(2π*fm*t) */
        double beta = m_modParams.modDepth * m_params.frequency /
            qMax(1e-10, m_modParams.modFrequency);
        double fmPhase = m_params.phase + 2.0 * M_PI * m_params.frequency * t
            + beta * qSin(2.0 * M_PI * m_modParams.modFrequency * t);

        /* 用FM相位重算基础波形 */
        switch (m_waveformType) {
        case WaveformType::Sine:
            baseValue = qSin(fmPhase);
            break;
        case WaveformType::Square:
            baseValue = (qSin(fmPhase) >= 0.0) ? 1.0 : -1.0;
            break;
        case WaveformType::Triangle: {
            double p = fmPhase / (2.0 * M_PI);
            p = p - qFloor(p);
            baseValue = 4.0 * qAbs(p - 0.5) - 1.0;
            break;
        }
        case WaveformType::Sawtooth: {
            double p = fmPhase / (2.0 * M_PI);
            p = p - qFloor(p);
            baseValue = 2.0 * p - 1.0;
            break;
        }
        default:
            baseValue = qSin(fmPhase);
            break;
        }
    } else {
        baseValue = computeBase(t);
        baseValue = applyModulation(baseValue, t);
    }

    return m_params.amplitude * baseValue + m_params.offset;
}

QVector<double> SignalGenerator::generate(int sampleCount)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> samples;
    samples.reserve(sampleCount);

    double dt = 1.0 / m_params.sampleRate;
    int progressInterval = qMax(1, sampleCount / 100);

    for (int i = 0; i < sampleCount; ++i) {
        double t = static_cast<double>(i) * dt;
        samples.append(sampleAt(t));

        if ((i + 1) % progressInterval == 0) {
            emit generationProgress(static_cast<double>(i + 1) / sampleCount);
        }
    }

    /* 更新统计 */
    ++m_stats.totalGenerated;
    m_stats.totalSamplesGenerated += sampleCount;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTime = m_timeSum / m_stats.totalGenerated;

    emit signalGenerated(samples);
    return samples;
}

QVector<double> SignalGenerator::generateDuration(double durationSec)
{
    int sampleCount = static_cast<int>(durationSec * m_params.sampleRate);
    return generate(sampleCount);
}

void SignalGenerator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
