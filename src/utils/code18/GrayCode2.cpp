/**
 * @file GrayCode2.cpp
 * @brief 格雷码工具实现 — 二进制/格雷码转换与序列生成
 */

#include "utils/code18/GrayCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
GrayCode2::GrayCode2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 二进制转格雷码 @param binary 二进制值 @return 格雷码值 */
quint64 GrayCode2::binaryToGray(quint64 binary)
{
    QElapsedTimer timer;
    timer.start();

    quint64 gray = binary ^ (binary >> 1);

    /* 更新统计 */
    ++m_stats.totalConversions;
    int bits = 0;
    quint64 tmp = binary;
    while (tmp) { ++bits; tmp >>= 1; }
    if (static_cast<quint64>(bits) > m_stats.maxBitsUsed) {
        m_stats.maxBitsUsed = bits;
    }
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalConversions
                             + m_stats.totalGenerations);

    emit conversionDone(binary, gray);
    return gray;
}

/** @brief 格雷码转二进制 @param gray 格雷码值 @return 二进制值 */
quint64 GrayCode2::grayToBinary(quint64 gray)
{
    QElapsedTimer timer;
    timer.start();

    quint64 binary = gray;
    /* 逐位异或还原: 每次将已还原的高位向右传播 */
    for (quint64 mask = gray >> 1; mask != 0; mask >>= 1) {
        binary ^= mask;
    }

    /* 更新统计 */
    ++m_stats.totalConversions;
    int bits = 0;
    quint64 tmp = gray;
    while (tmp) { ++bits; tmp >>= 1; }
    if (static_cast<quint64>(bits) > m_stats.maxBitsUsed) {
        m_stats.maxBitsUsed = bits;
    }
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalConversions
                             + m_stats.totalGenerations);

    emit conversionDone(gray, binary);
    return binary;
}

/** @brief 生成n位格雷码完整序列 @param n 位宽 @return 格雷码序列 */
QVector<quint64> GrayCode2::generateSequence(int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || n > 63) return {};

    int size = 1 << n;
    QVector<quint64> sequence;
    sequence.reserve(size);

    /* 镜像反射法生成格雷码序列 */
    sequence.append(0);
    for (int i = 0; i < n; ++i) {
        int currentSize = sequence.size();
        /* 在前面添加 2^i 个值，每个为当前值加上 2^i 前缀 */
        for (int j = currentSize - 1; j >= 0; --j) {
            sequence.append(sequence[j] | (1ULL << i));
        }
    }

    /* 更新统计 */
    ++m_stats.totalGenerations;
    m_stats.totalCodesGenerated += size;
    if (static_cast<quint64>(n) > m_stats.maxBitsUsed) {
        m_stats.maxBitsUsed = n;
    }
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalConversions
                             + m_stats.totalGenerations);

    emit sequenceGenerated(n, size);
    return sequence;
}

/** @brief 生成循环格雷码(首尾汉明距离为1) @param n 位宽 @return 循环格雷码序列 */
QVector<quint64> GrayCode2::generateCyclic(int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || n > 63) return {};

    /* 标准二进制反射格雷码天然首尾汉明距离为1 */
    QVector<quint64> seq = generateSequence(n);

    /* 验证首尾距离 */
    int dist = grayDistance(seq.first(), seq.last(), n);

    /* 更新统计 */
    ++m_stats.totalGenerations;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalConversions
                             + m_stats.totalGenerations);

    emit sequenceGenerated(n, seq.size());
    return seq;
}

/** @brief 计算格雷码汉明距离 @param grayA 码A @param grayB 码B @param bits 位宽 @return 距离 */
int GrayCode2::grayDistance(quint64 grayA, quint64 grayB, int bits)
{
    QElapsedTimer timer;
    timer.start();

    /* 格雷码距离 = 先转二进制再求汉明距离 */
    quint64 binA = grayToBinary(grayA);
    quint64 binB = grayToBinary(grayB);
    int dist = popcount(binA ^ binB);

    /* 截断到有效位 */
    quint64 mask = (bits >= 64) ? ~0ULL : ((1ULL << bits) - 1);
    quint64 xorVal = (grayA ^ grayB) & mask;
    dist = popcount(xorVal);

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalConversions
                             + m_stats.totalGenerations);
    return dist;
}

/** @brief 查找格雷码在序列中的位置 @param sequence 序列 @param gray 目标码 @return 索引 */
int GrayCode2::findGrayIndex(const QVector<quint64>& sequence, quint64 gray)
{
    QElapsedTimer timer;
    timer.start();

    /* 使用格雷码二进制索引特性优化查找:
     * 格雷码序列的第i个元素等于 i ^ (i >> 1)
     * 因此索引i = 格雷码转二进制(gray) */
    int nBits = 0;
    quint64 maxVal = 0;
    for (auto code : sequence) {
        if (code > maxVal) maxVal = code;
    }
    while ((1ULL << nBits) <= maxVal) ++nBits;

    /* 如果序列是标准反射格雷码，可以直接通过公式计算索引 */
    quint64 binaryIdx = grayToBinary(gray);
    if (binaryIdx < static_cast<quint64>(sequence.size())
        && sequence[static_cast<int>(binaryIdx)] == gray) {
        return static_cast<int>(binaryIdx);
    }

    /* 回退到线性查找 */
    for (int i = 0; i < sequence.size(); ++i) {
        if (sequence[i] == gray) return i;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalConversions
                             + m_stats.totalGenerations);
    return -1;
}

/** @brief 验证序列是否为合法格雷码序列 @param sequence 序列 @param bits 位宽 @return 合法性 */
bool GrayCode2::validateGraySequence(const QVector<quint64>& sequence,
                                     int bits)
{
    QElapsedTimer timer;
    timer.start();

    if (sequence.size() < 2) return true;
    quint64 mask = (bits >= 64) ? ~0ULL : ((1ULL << bits) - 1);

    /* 检查1: 序列长度应为 2^bits */
    int expectedSize = (bits < 31) ? (1 << bits) : sequence.size();
    if (bits < 31 && sequence.size() != expectedSize) return false;

    /* 检查2: 相邻元素汉明距离必须恰好为1 */
    for (int i = 1; i < sequence.size(); ++i) {
        quint64 diff = (sequence[i] ^ sequence[i - 1]) & mask;
        if (popcount(diff) != 1) return false;
    }

    /* 检查3: 所有码字互不相同 */
    QVector<quint64> sorted = sequence;
    std::sort(sorted.begin(), sorted.end());
    for (int i = 1; i < sorted.size(); ++i) {
        if (sorted[i] == sorted[i - 1]) return false;
    }

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalConversions
                             + m_stats.totalGenerations);
    return true;
}

/** @brief 重置统计 */
void GrayCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 计算popcount @param v 值 @return 1的位数 */
int GrayCode2::popcount(quint64 v) const
{
    int count = 0;
    while (v) {
        count += static_cast<int>(v & 1);
        v >>= 1;
    }
    return count;
}
