/**
 * @file GrayCodeGenerator.cpp
 * @brief Gray码(格雷码)生成器实现
 */

#include "GrayCodeGenerator.h"
#include <QElapsedTimer>

GrayCodeGenerator::GrayCodeGenerator(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

QVector<int> GrayCodeGenerator::generate(int bits) const
{
    QElapsedTimer timer;
    timer.start();

    int n = 1 << bits;
    QVector<int> codes(n);
    for (int i = 0; i < n; ++i)
        codes[i] = binaryToGray(i);

    m_stats.totalGenerated++;
    m_timeSum += timer.elapsed();
    int total = m_stats.totalGenerated + m_stats.totalConversions;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    emit sequenceGenerated(bits, n);
    return codes;
}

int GrayCodeGenerator::binaryToGray(int binary) const
{
    return binary ^ (binary >> 1);
}

int GrayCodeGenerator::grayToBinary(int gray) const
{
    int binary = gray;
    while (gray >>= 1)
        binary ^= gray;
    return binary;
}

QVector<int> GrayCodeGenerator::batchToGray(const QVector<int>& values) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    result.reserve(values.size());
    for (int v : values)
        result.append(binaryToGray(v));

    m_stats.totalConversions += values.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalGenerated + m_stats.totalConversions;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

QVector<int> GrayCodeGenerator::batchToBinary(const QVector<int>& values) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    result.reserve(values.size());
    for (int v : values)
        result.append(grayToBinary(v));

    m_stats.totalConversions += values.size();
    m_timeSum += timer.elapsed();
    int total = m_stats.totalGenerated + m_stats.totalConversions;
    if (total > 0) m_stats.avgProcessingTimeMs = m_timeSum / total;

    return result;
}

int GrayCodeGenerator::hammingDistance(int g1, int g2) const
{
    int xorVal = g1 ^ g2;
    int dist = 0;
    while (xorVal) {
        dist += xorVal & 1;
        xorVal >>= 1;
    }
    return dist;
}

int GrayCodeGenerator::grayCodeAt(int bits, int step) const
{
    if (step < 0 || step >= (1 << bits)) return 0;
    return binaryToGray(step);
}

QString GrayCodeGenerator::toString(int gray, int bits) const
{
    QString str;
    for (int i = bits - 1; i >= 0; --i)
        str += (gray & (1 << i)) ? QLatin1Char('1') : QLatin1Char('0');
    return str;
}

GrayCodeGenerator::Stats GrayCodeGenerator::stats() const { return m_stats; }

void GrayCodeGenerator::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
