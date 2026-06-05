/**
 * @file Demodulator.cpp
 * @brief 解调器引擎实现 — AM/FM/PM/ASK/FSK/PSK解调
 */

#include "utils/modulation/Demodulator.h"

#include <QtMath>
#include <cmath>

Demodulator::Demodulator(QObject* parent)
    : QObject(parent), m_type(ModType::AM), m_snrSum(0.0) {}

void Demodulator::setModType(ModType type) { m_type = type; }
void Demodulator::setConfig(const Config& config) { m_config = config; }

/** @brief 解调 @param data 信号 @return 解调输出 */
QVector<double> Demodulator::demodulate(const QVector<double>& data)
{
    if (data.isEmpty()) return {};

    QVector<double> result;
    switch (m_type) {
    case ModType::AM: result = demodAM(data); break;
    case ModType::FM: result = demodFM(data); break;
    case ModType::PM: result = demodPM(data); break;
    case ModType::ASK:
    case ModType::FSK:
    case ModType::PSK:
        result = demodAM(data); /* 数字调制回退到包络 */
        break;
    }

    m_stats.totalSamplesDemod += static_cast<quint64>(data.size());
    emit demodulationComplete(data.size());
    return result;
}

/** @brief 解码符号 @param data 信号 @return 比特 */
QVector<int> Demodulator::decodeSymbols(const QVector<double>& data)
{
    QVector<int> bits;
    switch (m_type) {
    case ModType::ASK: bits = decodeASK(data); break;
    case ModType::FSK: bits = decodeFSK(data); break;
    case ModType::PSK: bits = decodePSK(data); break;
    default: bits = decodeASK(data); break;
    }

    m_stats.totalSymbolsDecoded += static_cast<quint64>(bits.size());
    return bits;
}

void Demodulator::resetStatistics()
{
    m_stats = Stats{};
    m_snrSum = 0.0;
}

/** @brief AM包络检测 @param data 信号 @return 包络 */
QVector<double> Demodulator::demodAM(const QVector<double>& data) const
{
    QVector<double> envelope;
    envelope.reserve(data.size());

    /* 简单包络检测: 取绝对值后低通滤波 */
    int winSize = qMax(3, static_cast<int>(m_config.sampleRate / m_config.carrierFreq));
    double sum = 0.0;
    QVector<double> absData;
    absData.reserve(data.size());
    for (double v : data) absData.append(qAbs(v));

    for (int i = 0; i < absData.size(); ++i) {
        sum += absData[i];
        if (i >= winSize) sum -= absData[i - winSize];
        envelope.append(sum / qMin(i + 1, winSize));
    }
    return envelope;
}

/** @brief FM鉴频 @param data 信号 @return 瞬时频率 */
QVector<double> Demodulator::demodFM(const QVector<double>& data) const
{
    QVector<double> freq;
    freq.reserve(data.size());

    for (int i = 1; i < data.size(); ++i) {
        /* 瞬时相位差分 = 瞬时频率 */
        double phaseDiff = std::atan2(data[i], data[i - 1] + 1e-20)
                         - std::atan2(data[i - 1], data[i] + 1e-20);
        freq.append(phaseDiff * m_config.sampleRate / (2.0 * M_PI));
    }
    if (!freq.isEmpty()) freq.prepend(freq.first());
    return freq;
}

/** @brief PM相位检测 @param data 信号 @return 瞬时相位 */
QVector<double> Demodulator::demodPM(const QVector<double>& data) const
{
    QVector<double> phase;
    phase.reserve(data.size());

    /* Hilbert近似: 使用延迟线 */
    int delay = qMax(1, static_cast<int>(m_config.sampleRate /
        (4.0 * m_config.carrierFreq)));
    for (int i = 0; i < data.size(); ++i) {
        int j = qBound(0, i + delay, data.size() - 1);
        double ph = std::atan2(data[j], data[i] + 1e-20);
        phase.append(ph);
    }
    return phase;
}

/** @brief ASK解码 @param data 信号 @return 比特 */
QVector<int> Demodulator::decodeASK(const QVector<double>& data) const
{
    QVector<double> env = demodAM(data);
    if (env.isEmpty()) return {};

    /* 计算阈值 */
    double maxVal = *std::max_element(env.constBegin(), env.constEnd());
    double threshold = maxVal / 2.0;

    /* 按符号率采样 */
    int samplesPerSymbol = qMax(1, static_cast<int>(
        m_config.sampleRate / m_config.symbolRate));
    QVector<int> bits;
    for (int i = samplesPerSymbol / 2; i < env.size(); i += samplesPerSymbol) {
        bits.append(env[i] > threshold ? 1 : 0);
    }
    return bits;
}

/** @brief FSK解码 @param data 信号 @return 比特 */
QVector<int> Demodulator::decodeFSK(const QVector<double>& data) const
{
    /* 简单能量比较法 */
    int samplesPerSymbol = qMax(1, static_cast<int>(
        m_config.sampleRate / m_config.symbolRate));
    QVector<int> bits;

    for (int i = 0; i + samplesPerSymbol <= data.size(); i += samplesPerSymbol) {
        /* 计算过零率作为频率估计 */
        int zeroCross = 0;
        for (int j = i + 1; j < i + samplesPerSymbol; ++j) {
            if ((data[j] >= 0) != (data[j - 1] >= 0)) ++zeroCross;
        }
        double estFreq = static_cast<double>(zeroCross) / 2.0
            * m_config.sampleRate / samplesPerSymbol;
        double midFreq = (m_config.freqMark + m_config.freqSpace) / 2.0;
        bits.append(estFreq > midFreq ? 1 : 0);
    }
    return bits;
}

/** @brief PSK解码 @param data 信号 @return 比特 */
QVector<int> Demodulator::decodePSK(const QVector<double>& data) const
{
    QVector<double> ph = demodPM(data);
    int samplesPerSymbol = qMax(1, static_cast<int>(
        m_config.sampleRate / m_config.symbolRate));
    QVector<int> bits;

    for (int i = samplesPerSymbol / 2; i < ph.size(); i += samplesPerSymbol) {
        /* BPSK: 相位>0为1, <0为0 */
        bits.append(ph[i] > 0 ? 1 : 0);
    }
    return bits;
}
