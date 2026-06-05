/**
 * @file AdaptiveDpcm.cpp
 * @brief 自适应DPCM编解码实现
 */

#include "utils/pcm/AdaptiveDpcm.h"

#include <QElapsedTimer>
#include <QDataStream>
#include <QtMath>
#include <cstring>

AdaptiveDpcm::AdaptiveDpcm(QObject* parent)
    : QObject(parent) {}

QByteArray AdaptiveDpcm::encode(const QVector<double>& samples)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;

    if (samples.isEmpty()) {
        m_timeSum += static_cast<double>(timer.elapsed());
        m_stats.avgProcessingTimeMs =
            (m_stats.totalEncoded + m_stats.totalDecoded > 0) ?
            m_timeSum / (m_stats.totalEncoded + m_stats.totalDecoded) : 0.0;
        return result;
    }

    /* 计算每个编码所需的位数 */
    int bitsPerCode = qMax(1, static_cast<int>(qCeil(qLn(m_quantLevels) /
                                                       qLn(2.0))));

    /* 头部: 量化级数(1B) + 第一个样本(double, 8B) */
    result.append(static_cast<char>(m_quantLevels));
    double firstSample = samples[0];
    result.append(reinterpret_cast<const char*>(&firstSample), 8);

    /* 重置步长 */
    m_stepSize = 1.0;

    /* 计算信号范围以初始化步长 */
    double minVal = samples[0], maxVal = samples[0];
    for (int i = 1; i < samples.size(); ++i) {
        if (samples[i] < minVal) minVal = samples[i];
        if (samples[i] > maxVal) maxVal = samples[i];
    }
    double range = maxVal - minVal;
    if (range > 1e-15) {
        m_stepSize = range / m_quantLevels;
    }

    /* 编码差分 */
    int bitPos = 0;
    double prev = samples[0];

    for (int i = 1; i < samples.size(); ++i) {
        double delta = samples[i] - prev;
        int code = quantize(delta);
        writeBits(result, code, bitsPerCode, bitPos);
        prev = prev + dequantize(code);
        updateStepSize(code);
    }

    /* 填充最后一个字节 */
    if (bitPos > 0) {
        /* 已在writeBits中追加字节 */
    }

    /* 附加样本数（4B小端）用于解码校验 */
    quint32 count = static_cast<quint32>(samples.size());
    result.append(reinterpret_cast<const char*>(&count), 4);

    m_stats.totalEncoded++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncoded + m_stats.totalDecoded);

    emit encodingCompleted(samples.size() * 8, result.size());
    return result;
}

QVector<double> AdaptiveDpcm::decode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;

    /* 最小头部: 1(quantLevels) + 8(firstSample) + 4(count) = 13 */
    if (data.size() < 13) {
        m_timeSum += static_cast<double>(timer.elapsed());
        return result;
    }

    int pos = 0;

    /* 读取量化级数 */
    int levels = static_cast<unsigned char>(data[pos++]);
    int bitsPerCode = qMax(1, static_cast<int>(qCeil(qLn(levels) / qLn(2.0))));

    /* 读取第一个样本 */
    double firstSample;
    std::memcpy(&firstSample, data.constData() + pos, 8);
    pos += 8;

    /* 读取尾部样本计数 */
    quint32 count;
    std::memcpy(&count, data.constData() + data.size() - 4, 4);

    result.reserve(count);
    result.append(firstSample);

    /* 重置步长（与编码一致） */
    int savedLevels = m_quantLevels;
    m_quantLevels = levels;
    m_stepSize = 1.0;

    if (count > 1) {
        /* 初始化步长：先粗略估计 */
        m_stepSize = 0.1;
    }

    int bitPos = 0;
    double prev = firstSample;
    int dataEnd = data.size() - 4;

    for (quint32 i = 1; i < count; ++i) {
        int code = readBits(data, bitsPerCode, bitPos);
        /* readBits内部用pos, 我们在这里用dataEnd+pos */
        double delta = dequantize(code);
        prev = prev + delta;
        result.append(prev);
        updateStepSize(code);
    }

    m_quantLevels = savedLevels;

    m_stats.totalDecoded++;
    m_timeSum += static_cast<double>(timer.elapsed());
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncoded + m_stats.totalDecoded);

    return result;
}

void AdaptiveDpcm::setQuantLevels(int levels)
{
    m_quantLevels = qBound(2, levels, 256);
}

int AdaptiveDpcm::quantize(double delta)
{
    double step = qMax(m_stepSize, m_minStep);
    int halfLevels = m_quantLevels / 2;

    /* 映射到 [0, quantLevels-1] */
    int code = static_cast<int>(delta / step) + halfLevels;
    return qBound(0, code, m_quantLevels - 1);
}

double AdaptiveDpcm::dequantize(int code)
{
    double step = qMax(m_stepSize, m_minStep);
    int halfLevels = m_quantLevels / 2;
    return (code - halfLevels) * step;
}

void AdaptiveDpcm::updateStepSize(int code)
{
    int halfLevels = m_quantLevels / 2;
    int magnitude = qAbs(code - halfLevels);

    /* 大幅差值 → 放大步长; 小幅差值 → 缩小步长 */
    if (magnitude > halfLevels / 2) {
        m_stepSize *= STEP_SCALE_UP;
    } else {
        m_stepSize *= STEP_SCALE_DOWN;
    }
    m_stepSize = qBound(m_minStep, m_stepSize, m_maxStep);
}

void AdaptiveDpcm::writeBits(QByteArray& buf, int value, int bitCount,
                             int& bitPos) const
{
    static int currentByte = 0;
    static int bitsFilled = 0;

    if (bitPos == 0) {
        currentByte = 0;
        bitsFilled = 0;
    }

    for (int i = bitCount - 1; i >= 0; --i) {
        int bit = (value >> i) & 1;
        currentByte |= (bit << (7 - bitsFilled));
        bitsFilled++;
        bitPos++;

        if (bitsFilled == 8) {
            buf.append(static_cast<char>(currentByte));
            currentByte = 0;
            bitsFilled = 0;
        }
    }
}

int AdaptiveDpcm::readBits(const QByteArray& buf, int bitCount,
                           int& bitPos) const
{
    int value = 0;
    static int byteIdx = 0;

    if (bitPos == 0) {
        byteIdx = 9; /* 跳过头部: 1 + 8 */
    }

    for (int i = 0; i < bitCount; ++i) {
        if (byteIdx < buf.size() - 4) {
            int bit = (buf[byteIdx] >> (7 - (bitPos % 8))) & 1;
            value = (value << 1) | bit;
            bitPos++;
            if (bitPos % 8 == 0) byteIdx++;
        }
    }
    return value;
}

void AdaptiveDpcm::resetStatistics()
{
    m_stats   = Stats{};
    m_timeSum = 0.0;
}
