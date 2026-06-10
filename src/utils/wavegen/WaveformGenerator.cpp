/**
 * @file WaveformGenerator.cpp
 * @brief 流式波形发生器实现
 * @author EmbedDebug Team
 * @date 2026-06-06
 */

#include "utils/wavegen/WaveformGenerator.h"

#include <QDataStream>
#include <QIODevice>
#include <QtMath>

#include <algorithm>
#include <chrono>
#include <cmath>
#include <random>

// ══════════════════════════════════════════════
// 构造 / 析构
// ══════════════════════════════════════════════

WaveGenEngine::WaveGenEngine(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
{
    m_timer->setSingleShot(false);
    connect(m_timer, &QTimer::timeout,
            this, &WaveGenEngine::onStreamingTick);
}

WaveGenEngine::~WaveGenEngine()
{
    stopStreaming();
}

// ══════════════════════════════════════════════
// 参数配置
// ══════════════════════════════════════════════

void WaveGenEngine::setParams(const WaveGen::WaveGenParams &params)
{
    m_params = params;
    m_phaseAccumulator = params.phase;
    m_sampleIndex = 0;
    emit paramsChanged();
}

WaveGen::WaveGenParams WaveGenEngine::params() const
{
    return m_params;
}

void WaveGenEngine::setChunkSize(int size)
{
    m_chunkSize = qBound(1, size, 65536);
}

int WaveGenEngine::chunkSize() const
{
    return m_chunkSize;
}

// ══════════════════════════════════════════════
// 流式输出控制
// ══════════════════════════════════════════════

void WaveGenEngine::startStreaming(int intervalMs)
{
    if (m_streaming) {
        return;
    }
    m_streaming = true;
    m_timer->start(qMax(1, intervalMs));
}

void WaveGenEngine::stopStreaming()
{
    if (!m_streaming) {
        return;
    }
    m_streaming = false;
    m_timer->stop();
}

bool WaveGenEngine::isStreaming() const
{
    return m_streaming;
}

// ══════════════════════════════════════════════
// 单次生成
// ══════════════════════════════════════════════

QVector<double> WaveGenEngine::generateRaw(int count)
{
    QVector<double> result;
    result.reserve(count);

    for (int i = 0; i < count; ++i) {
        result.append(computeSample(m_sampleIndex));
        ++m_sampleIndex;
    }

    m_stats.totalGenerations++;
    m_stats.totalSamples += static_cast<quint64>(count);
    return result;
}

QByteArray WaveGenEngine::quantize(const QVector<double> &raw) const
{
    return toBytes(raw);
}

// ══════════════════════════════════════════════
// 流式定时器回调
// ══════════════════════════════════════════════

void WaveGenEngine::onStreamingTick()
{
    if (!m_streaming) {
        return;
    }
    QVector<double> raw = generateRaw(m_chunkSize);
    QByteArray bytes = toBytes(raw);
    m_stats.streamingChunks++;
    m_stats.totalBytesOutput += static_cast<quint64>(bytes.size());
    emit dataGenerated(bytes);
}

// ══════════════════════════════════════════════
// 核心采样计算
// ══════════════════════════════════════════════

double WaveGenEngine::computeSample(int index)
{
    const double sr = m_params.sampleRate;
    const double t = static_cast<double>(index) / sr;
    const double freq = m_params.frequency;
    const double amp = m_params.amplitude;
    const double off = m_params.offset;
    const double phase = m_params.phase;

    double sample = 0.0;

    switch (m_params.type) {
    case WaveGen::WaveformType::Sine:
        sample = amp * qSin(2.0 * M_PI * freq * t + phase);
        break;

    case WaveGen::WaveformType::Square: {
        double p = qSin(2.0 * M_PI * freq * t + phase);
        double threshold = 2.0 * m_params.dutyCycle - 1.0;
        sample = amp * (p >= threshold ? 1.0 : -1.0);
        break;
    }

    case WaveGen::WaveformType::Triangle: {
        double period = 1.0 / freq;
        double tMod = std::fmod(t + phase / (2.0 * M_PI * freq), period);
        if (tMod < 0) tMod += period;
        double frac = tMod / period;
        if (frac < 0.5) {
            sample = amp * (4.0 * frac - 1.0);
        } else {
            sample = amp * (3.0 - 4.0 * frac);
        }
        break;
    }

    case WaveGen::WaveformType::Sawtooth: {
        double period = 1.0 / freq;
        double tMod = std::fmod(t + phase / (2.0 * M_PI * freq), period);
        if (tMod < 0) tMod += period;
        double frac = tMod / period;
        sample = amp * (2.0 * frac - 1.0);
        break;
    }

    case WaveGen::WaveformType::Noise: {
        static thread_local std::mt19937_64 gen(
            static_cast<quint64>(
                std::chrono::steady_clock::now().time_since_epoch().count()));
        static thread_local std::uniform_real_distribution<double> dist(-1.0, 1.0);
        sample = amp * dist(gen);
        break;
    }

    case WaveGen::WaveformType::Custom: {
        const auto &data = m_params.customData;
        if (data.size() < 2) {
            sample = 0.0;
            break;
        }
        double period = 1.0 / freq;
        double tMod = std::fmod(t, period);
        if (tMod < 0) tMod += period;
        double frac = tMod / period;
        double fidx = frac * (data.size() - 1);
        int lo = static_cast<int>(fidx);
        int hi = qMin(lo + 1, data.size() - 1);
        double alpha = fidx - lo;
        sample = amp * (data[lo] * (1.0 - alpha) + data[hi] * alpha);
        break;
    }
    }

    sample += off;
    sample = applyModulation(sample, t);
    return sample;
}

// ══════════════════════════════════════════════
// 调制
// ══════════════════════════════════════════════

double WaveGenEngine::applyModulation(double sample, double t)
{
    if (m_params.modType == WaveGen::ModulationType::None) {
        return sample;
    }
    double modSignal = qSin(2.0 * M_PI * m_params.modFrequency * t);
    double depth = m_params.modDepth;

    switch (m_params.modType) {
    case WaveGen::ModulationType::AM:
        return sample * (1.0 + depth * modSignal);
    case WaveGen::ModulationType::FM: {
        double phaseShift = depth * m_params.frequency * modSignal;
        return m_params.amplitude
               * qSin(2.0 * M_PI * m_params.frequency * t + phaseShift)
               + m_params.offset;
    }
    default:
        return sample;
    }
}

// ══════════════════════════════════════════════
// 量化 + 字节序转换
// ══════════════════════════════════════════════

QByteArray WaveGenEngine::toBytes(const QVector<double> &raw) const
{
    QByteArray result;
    QDataStream stream(&result, QIODevice::WriteOnly);

    if (m_params.endian == WaveGen::Endianness::BigEndian) {
        stream.setByteOrder(QDataStream::BigEndian);
    } else {
        stream.setByteOrder(QDataStream::LittleEndian);
    }

    const double amp = m_params.amplitude + qAbs(m_params.offset);
    const double maxVal = (amp > 0.0) ? amp : 1.0;

    for (double val : raw) {
        switch (m_params.bitDepth) {
        case WaveGen::BitDepth::Bits8: {
            quint8 byte = static_cast<quint8>(
                qBound(0.0, ((val / maxVal) + 1.0) * 0.5 * 255.0, 255.0));
            stream << byte;
            break;
        }
        case WaveGen::BitDepth::Bits16: {
            qint16 s16 = static_cast<qint16>(
                qBound(-32768.0, (val / maxVal) * 32767.0, 32767.0));
            stream << s16;
            break;
        }
        case WaveGen::BitDepth::Bits32: {
            qint32 s32 = static_cast<qint32>(
                qBound(-2147483648.0, (val / maxVal) * 2147483647.0,
                       2147483647.0));
            stream << s32;
            break;
        }
        }
    }

    return result;
}
