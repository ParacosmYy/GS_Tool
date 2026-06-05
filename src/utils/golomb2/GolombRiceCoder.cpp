/**
 * @file GolombRiceCoder.cpp
 * @brief Golomb-Rice编解码器实现
 */

#include "GolombRiceCoder.h"
#include <QElapsedTimer>
#include <cmath>

GolombRiceCoder::GolombRiceCoder(int m, QObject* parent)
    : QObject(parent)
    , m_m(qMax(1, m))
    , m_k(0)
    , m_timeSum(0.0)
{
    /* 确保M是2的幂 */
    int val = 1;
    while (val < m_m) { val <<= 1; m_k++; }
    m_m = val;
}

QByteArray GolombRiceCoder::encode(quint32 value)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    quint32 q = value >> m_k;
    quint32 r = value & (static_cast<quint32>(m_m) - 1);

    /* 一元编码q: q个1后跟1个0 */
    int totalBits = q + 1 + m_k;
    result.resize((totalBits + 7) / 8, 0);

    int bitPos = 0;
    auto writeBit = [&](int b) {
        if (b) result[bitPos / 8] |= (1 << (7 - bitPos % 8));
        bitPos++;
    };

    for (quint32 i = 0; i < q; ++i) writeBit(1);
    writeBit(0);

    /* Rice余数: m_k位二进制 */
    for (int i = m_k - 1; i >= 0; --i)
        writeBit((r >> i) & 1);

    m_stats.totalEncoded++;
    m_stats.totalBitsIn += 32;
    m_stats.totalBitsOut += totalBits;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

QByteArray GolombRiceCoder::encodeBatch(const QVector<quint32>& values)
{
    QElapsedTimer timer;
    timer.start();

    QByteArray result;
    int totalBits = 0;

    /* 预估总比特数 */
    for (quint32 v : values) totalBits += encodedBits(v);
    result.resize((totalBits + 7) / 8 + 1, 0);

    int bitPos = 0;
    auto writeBit = [&](int b) {
        while (bitPos / 8 >= result.size()) result.append('\0');
        if (b) result[bitPos / 8] |= (1 << (7 - bitPos % 8));
        bitPos++;
    };

    for (quint32 value : values) {
        quint32 q = value >> m_k;
        quint32 r = value & (static_cast<quint32>(m_m) - 1);

        for (quint32 i = 0; i < q; ++i) writeBit(1);
        writeBit(0);

        for (int i = m_k - 1; i >= 0; --i)
            writeBit((r >> i) & 1);
    }

    m_stats.totalEncoded++;
    m_stats.totalBitsIn += values.size() * 32;
    m_stats.totalBitsOut += totalBits;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit encodingCompleted(values.size(), values.size() * 32, totalBits);
    return result;
}

QVector<quint32> GolombRiceCoder::decode(const QByteArray& data, int count)
{
    QElapsedTimer timer;
    timer.start();

    QVector<quint32> result;
    int bytePos = 0, bitIdx = 7;
    auto readBit = [&]() -> int {
        if (bytePos >= data.size()) return 0;
        int b = (data[bytePos] >> bitIdx) & 1;
        bitIdx--;
        if (bitIdx < 0) { bitIdx = 7; bytePos++; }
        return b;
    };

    for (int c = 0; c < count; ++c) {
        /* 解码一元部分 */
        quint32 q = 0;
        while (readBit() == 1) q++;

        /* 解码Rice余数 */
        quint32 r = 0;
        for (int i = 0; i < m_k; ++i)
            r = (r << 1) | readBit();

        result.append((q << m_k) | r);
    }

    m_stats.totalDecoded++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalEncoded + m_stats.totalDecoded;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

int GolombRiceCoder::optimalM(const QVector<quint32>& values)
{
    if (values.isEmpty()) return 1;

    double sum = 0.0;
    for (quint32 v : values) sum += v;
    double mean = sum / values.size();

    if (mean <= 0.0) return 1;

    /* 几何分布最优M ≈ ceil(-ln2 / ln(1-p)) 简化为 ceil(mean * 0.69) */
    int m = qMax(1, static_cast<int>(std::ceil(mean * 0.693)));

    /* 对齐到2的幂 */
    int power = 1;
    while (power < m) power <<= 1;
    return power;
}

void GolombRiceCoder::setM(int m)
{
    m_m = qMax(1, m);
    m_k = 0;
    int val = 1;
    while (val < m_m) { val <<= 1; m_k++; }
    m_m = val;
}

int GolombRiceCoder::m() const { return m_m; }

int GolombRiceCoder::encodedBits(quint32 value) const
{
    quint32 q = value >> m_k;
    return q + 1 + m_k;
}

GolombRiceCoder::Stats GolombRiceCoder::stats() const { return m_stats; }

void GolombRiceCoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
