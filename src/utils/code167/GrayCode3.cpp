/**
 * @file GrayCode3.cpp
 * @brief GrayCode3 实现
 *
 * 实现格雷码编码/解码/排名/逆排名/序列生成/汉明距离。
 */

#include "utils/code167/GrayCode3.h"

#include <QElapsedTimer>
#include <QtMath>

GrayCode3::GrayCode3(QObject *parent)
    : QObject(parent)
{
}

GrayCode3::~GrayCode3() = default;

void GrayCode3::setBitWidth(int bits)
{
    m_bits = qBound(1, bits, 31);
}

quint32 GrayCode3::encode(quint32 binary) const
{
    /* Gray = binary XOR (binary >> 1) */
    return binary ^ (binary >> 1);
}

quint32 GrayCode3::decode(quint32 gray) const
{
    /* Inverse Gray: XOR cascade */
    quint32 mask = gray;
    for (int i = 1; i < m_bits; ++i)
        mask ^= (mask >> i);
    /* Full unroll for up to 32 bits */
    quint32 inv = gray;
    inv ^= (inv >> 1);
    inv ^= (inv >> 2);
    inv ^= (inv >> 4);
    inv ^= (inv >> 8);
    inv ^= (inv >> 16);
    return inv;
}

quint32 GrayCode3::rank(quint32 gray) const
{
    /* Rank = decoded binary value (position in binary-reflected Gray code) */
    return decode(gray);
}

quint32 GrayCode3::unrank(quint32 r) const
{
    /* Unrank = encode the rank */
    return encode(r);
}

QVector<quint32> GrayCode3::generateSequence() const
{
    int n = 1 << m_bits;
    QVector<quint32> seq(n);
    for (int i = 0; i < n; ++i)
        seq[i] = encode(static_cast<quint32>(i));

    emit sequenceGenerated(n);
    return seq;
}

int GrayCode3::hammingDistance(quint32 a, quint32 b) const
{
    return popcount(a ^ b);
}

int GrayCode3::popcount(quint32 x)
{
    /* Brian Kernighan's bit count */
    int count = 0;
    while (x) {
        x &= (x - 1);
        count++;
    }
    return count;
}

void GrayCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
