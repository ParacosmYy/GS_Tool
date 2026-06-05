/**
 * @file ConvolutionalEncoder.cpp
 * @brief 卷积码编码器实现 — 多项式生成器
 */

#include "utils/code5/ConvolutionalEncoder.h"

#include <QElapsedTimer>
#include <QtMath>

ConvolutionalEncoder::ConvolutionalEncoder(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
    /* 默认配置: rate 1/2, K=7, 通用多项式 */
    m_config.constraintLength = 7;
    m_config.generators = {0171, 0133};  /* 八进制: 171=1111001, 133=1011011 */
    m_config.enableTermination = true;
    m_config.enablePuncturing = false;
    m_mask = (1 << (m_config.constraintLength - 1)) - 1;
}

void ConvolutionalEncoder::configure(const Config& config)
{
    m_config = config;
    m_mask = (1 << (config.constraintLength - 1)) - 1;
    m_state = 0;
}

QByteArray ConvolutionalEncoder::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    if (data.isEmpty()) return {};

    /* 将字节数据展开为比特数组 */
    QVector<quint8> bits;
    bits.reserve(data.size() * 8);
    for (int i = 0; i < data.size(); ++i) {
        quint8 byte = static_cast<quint8>(data[i]);
        for (int b = 7; b >= 0; --b) {
            bits.append((byte >> b) & 1);
        }
    }

    /* 卷积编码 */
    QVector<quint8> encoded = encodeBits(bits);

    /* 打孔 */
    if (m_config.enablePuncturing && !m_config.puncturePattern.isEmpty()) {
        encoded = applyPuncturing(encoded);
    }

    /* 将比特打包为字节数组 */
    QByteArray result;
    result.reserve((encoded.size() + 7) / 8);
    for (int i = 0; i < encoded.size(); i += 8) {
        quint8 byte = 0;
        for (int b = 0; b < 8 && (i + b) < encoded.size(); ++b) {
            byte |= (encoded[i + b] << (7 - b));
        }
        result.append(static_cast<char>(byte));
    }

    /* 更新统计 */
    m_stats.totalBlocksEncoded++;
    m_stats.totalInputBits += static_cast<int>(bits.size());
    m_stats.totalOutputBits += static_cast<int>(encoded.size());
    int nGenerators = m_config.generators.size();
    m_stats.codeRate = (nGenerators > 0 && !m_config.enablePuncturing)
        ? 1.0 / nGenerators : 0.0;

    m_timeSum += timer.elapsed();
    if (m_stats.totalBlocksEncoded > 0) {
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalBlocksEncoded;
    }

    emit encoded(static_cast<int>(bits.size()),
                 static_cast<int>(encoded.size()));
    return result;
}

QVector<quint8> ConvolutionalEncoder::encodeBits(const QVector<quint8>& bits)
{
    if (bits.isEmpty() || m_config.generators.isEmpty()) return {};

    QVector<quint8> inputBits = bits;

    /* 添加尾比特终止 */
    if (m_config.enableTermination) {
        inputBits = addTermination(inputBits);
    }

    QVector<quint8> output;
    output.reserve(inputBits.size() * m_config.generators.size());

    /* 逐比特编码 */
    for (int i = 0; i < inputBits.size(); ++i) {
        /* 将输入比特移入寄存器最高位 */
        m_state = ((m_state << 1) | (inputBits[i] & 1)) & m_mask;

        /* 对每个生成多项式计算输出 */
        for (quint32 gen : m_config.generators) {
            output.append(computeOutput(m_state, gen));
        }
    }

    return output;
}

QVector<quint8> ConvolutionalEncoder::addTermination(
    const QVector<quint8>& bits)
{
    QVector<quint8> result = bits;
    int tailBits = m_config.constraintLength - 1;

    /* 添加K-1个零比特, 使移位寄存器归零 */
    for (int i = 0; i < tailBits; ++i) {
        result.append(0);
    }
    return result;
}

QVector<quint8> ConvolutionalEncoder::applyPuncturing(
    const QVector<quint8>& encodedBits)
{
    if (m_config.puncturePattern.isEmpty()) return encodedBits;

    int patternLen = m_config.puncturePattern.size();
    QVector<quint8> result;
    result.reserve(encodedBits.size());

    int puncturedCount = 0;
    for (int i = 0; i < encodedBits.size(); ++i) {
        int patIdx = i % patternLen;
        if (patIdx < patternLen &&
            m_config.puncturePattern[patIdx] != 0) {
            result.append(encodedBits[i]);
        } else {
            ++puncturedCount;
        }
    }

    m_stats.totalPuncturedBits += puncturedCount;

    /* 更新有效码率 */
    int nGen = m_config.generators.size();
    if (nGen > 0 && result.size() > 0) {
        int keptPerPattern = 0;
        for (int i = 0; i < patternLen; ++i) {
            if (m_config.puncturePattern[i] != 0) ++keptPerPattern;
        }
        m_stats.codeRate = static_cast<double>(patternLen) /
                           (keptPerPattern * nGen);
    }

    return result;
}

quint32 ConvolutionalEncoder::encoderState() const
{
    return m_state;
}

void ConvolutionalEncoder::reset()
{
    m_state = 0;
}

QVector<quint8> ConvolutionalEncoder::branchOutput(quint32 state,
                                                    quint8 input) const
{
    quint32 newState = ((state << 1) | (input & 1)) & m_mask;
    QVector<quint8> output;
    output.reserve(m_config.generators.size());

    for (quint32 gen : m_config.generators) {
        output.append(computeOutput(newState, gen));
    }
    return output;
}

quint32 ConvolutionalEncoder::nextState(quint32 state, quint8 input) const
{
    return ((state << 1) | (input & 1)) & m_mask;
}

ConvolutionalEncoder::Config ConvolutionalEncoder::config() const
{
    return m_config;
}

ConvolutionalEncoder::Stats ConvolutionalEncoder::stats() const
{
    return m_stats;
}

void ConvolutionalEncoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

quint8 ConvolutionalEncoder::computeOutput(quint32 state,
                                           quint32 generator) const
{
    /* 将生成多项式与寄存器状态按位与, 计算1的个数(模2) */
    quint32 masked = state & generator;
    quint8 result = 0;
    while (masked) {
        result ^= (masked & 1);
        masked >>= 1;
    }
    return result;
}
