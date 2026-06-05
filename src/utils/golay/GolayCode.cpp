/**
 * @file GolayCode.cpp
 * @brief 扩展Golay码 (24,12) 实现
 */

#include "utils/golay/GolayCode.h"

#include <QElapsedTimer>
#include <algorithm>

GolayCode::GolayCode(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
    initGeneratorMatrix();
}

void GolayCode::initGeneratorMatrix()
{
    /* 扩展Golay码(24,12,8)的生成矩阵G = [I12 | P]
     * P矩阵为已知的12x12校验矩阵 */
    static const quint32 P[12] = {
        0b110111000101,
        0b101110001011,
        0b011100010111,
        0b111000101101,
        0b110001011011,
        0b100010110111,
        0b000101101111,
        0b001011011101,
        0b010110111001,
        0b101101110001,
        0b011011100011,
        0b111111111110
    };
    for (int i = 0; i < 12; ++i) {
        /* 每行: 12位数据位 | 12位校验位 */
        m_generatorMatrix[i] = (1u << (23 - i)) | (P[i] << 11);
        /* 重新排列: 数据位在高位, 校验位在低12位 */
        m_generatorMatrix[i] = ((1u << (11 - i)) << 12) | P[i];
    }
}

quint32 GolayCode::encode12Bit(quint32 data) const
{
    data &= 0xFFF; /* 取低12位 */
    quint32 codeword = data << 12;

    /* 计算校验位: data * P矩阵 */
    quint32 parity = 0;
    for (int i = 0; i < 12; ++i) {
        if (data & (1u << (11 - i))) {
            parity ^= (m_generatorMatrix[i] & 0xFFF);
        }
    }
    return codeword | parity;
}

quint32 GolayCode::syndrome(quint32 codeword) const
{
    /* 伴随式 = codeword * H^T, 简化为用生成矩阵校验 */
    quint32 data = (codeword >> 12) & 0xFFF;
    quint32 parity = codeword & 0xFFF;

    /* 重新计算校验位 */
    quint32 expected = 0;
    for (int i = 0; i < 12; ++i) {
        if (data & (1u << (11 - i))) {
            expected ^= (m_generatorMatrix[i] & 0xFFF);
        }
    }
    return parity ^ expected;
}

int GolayCode::popcount(quint32 x) const
{
    int count = 0;
    while (x) {
        count += x & 1;
        x >>= 1;
    }
    return count;
}

quint32 GolayCode::decode24Bit(quint32 codeword, bool& corrected)
{
    corrected = false;
    quint32 syn = syndrome(codeword);

    /* 无错误 */
    if (syn == 0) {
        return (codeword >> 12) & 0xFFF;
    }

    int synWeight = popcount(syn);

    /* 伴随式重量 <= 3: 错误在校验位 */
    if (synWeight <= 3) {
        corrected = true;
        quint32 fixed = codeword ^ syn;
        return (fixed >> 12) & 0xFFF;
    }

    /* 检查是否单bit错误在数据位+伴随式模式匹配 */
    for (int i = 0; i < 12; ++i) {
        quint32 rowParity = m_generatorMatrix[i] & 0xFFF;
        int weight = popcount(syn ^ rowParity);
        if (weight <= 2) {
            corrected = true;
            quint32 fixed = codeword ^ (1u << (23 - i));
            return (fixed >> 12) & 0xFFF;
        }
    }

    /* 尝试双bit错误纠正: 翻转一个数据位再检查 */
    for (int i = 0; i < 12; ++i) {
        quint32 flipped = codeword ^ (1u << (23 - i));
        quint32 syn2 = syndrome(flipped);
        if (popcount(syn2) <= 2) {
            corrected = true;
            quint32 fixed = flipped ^ syn2;
            return (fixed >> 12) & 0xFFF;
        }
    }

    /* 尝试翻转一个校验位 */
    for (int j = 0; j < 12; ++j) {
        quint32 flipped = codeword ^ (1u << (11 - j));
        quint32 syn2 = syndrome(flipped);
        if (popcount(syn2) <= 2) {
            corrected = true;
            quint32 fixed = flipped ^ syn2;
            return (fixed >> 12) & 0xFFF;
        }
    }

    /* 无法纠正,返回数据位(可能不正确) */
    return (codeword >> 12) & 0xFFF;
}

QByteArray GolayCode::encode(const QByteArray& data)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    int bitBuf = 0;
    int bitCount = 0;

    /* 将输入字节流按12bit分组编码为24bit码字 */
    for (int i = 0; i < data.size(); ++i) {
        for (int b = 7; b >= 0; --b) {
            bitBuf = (bitBuf << 1) | ((data[i] >> b) & 1);
            bitCount++;

            if (bitCount == 12) {
                quint32 codeword = encode12Bit(bitBuf & 0xFFF);
                /* 写入3字节(24bit) */
                result.append(static_cast<char>((codeword >> 16) & 0xFF));
                result.append(static_cast<char>((codeword >> 8) & 0xFF));
                result.append(static_cast<char>(codeword & 0xFF));
                bitBuf = 0;
                bitCount = 0;
            }
        }
    }

    /* 处理不足12bit的尾部(补零) */
    if (bitCount > 0) {
        quint32 codeword = encode12Bit(bitBuf & 0xFFF);
        result.append(static_cast<char>((codeword >> 16) & 0xFF));
        result.append(static_cast<char>((codeword >> 8) & 0xFF));
        result.append(static_cast<char>(codeword & 0xFF));
    }

    m_stats.totalEncoded++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    return result;
}

QByteArray GolayCode::decode(const QByteArray& received)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    bool anyCorrected = false;

    /* 每3字节(24bit)解码为12bit数据 */
    for (int i = 0; i + 2 < received.size(); i += 3) {
        quint32 codeword = (static_cast<quint8>(received[i]) << 16) |
                           (static_cast<quint8>(received[i + 1]) << 8) |
                           static_cast<quint8>(received[i + 2]);

        bool corrected = false;
        quint32 decoded = decode24Bit(codeword, corrected);
        if (corrected) anyCorrected = true;

        /* 将12bit输出为字节流 */
        result.append(static_cast<char>((decoded >> 4) & 0xFF));
        /* 低4bit留待下一轮 */
        int lowNibble = decoded & 0xF;
        /* 简化: 每个码字输出2字节(高8位+低4位补零) */
        result.append(static_cast<char>((lowNibble << 4) & 0xF0));
    }

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    double total = m_stats.totalEncoded + m_stats.totalDecoded;
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit decodingCompleted(anyCorrected);
    return result;
}

int GolayCode::hammingWeight(const QByteArray& codeword)
{
    int weight = 0;
    for (int i = 0; i < codeword.size(); ++i) {
        quint8 byte = static_cast<quint8>(codeword[i]);
        while (byte) {
            weight += byte & 1;
            byte >>= 1;
        }
    }
    return weight;
}

void GolayCode::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
