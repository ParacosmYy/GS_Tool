/**
 * @file GolombCoder.cpp
 * @brief Golomb-Rice编码器实现 — 几何分布整数压缩
 */

#include "utils/golomb/GolombCoder.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QRandomGenerator>

/** @brief 构造函数 @param parent 父对象 */
GolombCoder::GolombCoder(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 编码非负整数序列
 *  @param values 非负整数数组
 *  @param m      Golomb参数
 *  @return 编码后的字节流 */
QByteArray GolombCoder::encode(const QVector<int>& values, int m)
{
    QElapsedTimer timer;
    timer.start();

    if (values.isEmpty() || m <= 0) {
        return QByteArray();
    }

    /* 预估编码大小: 每个值平均约log2(m)+m/ln(2)位 */
    int estimatedBits = values.size() * (qCeil(qLn(m + 1) / qLn(2)) + m + 8);
    int estimatedBytes = (estimatedBits + 7) / 8 + 4;

    QByteArray bitBuffer(estimatedBytes, 0);
    int bitPos = 0;

    /* 编码每个值 */
    for (int val : values) {
        if (val < 0) val = 0; /* 确保非负 */
        encodeSingle(val, m, bitBuffer, bitPos);
    }

    /* 截取有效数据 */
    int totalBytes = (bitPos + 7) / 8;
    QByteArray result = bitBuffer.left(totalBytes);

    /* 计算压缩比 */
    double originalBits = static_cast<double>(values.size()) * 32.0;
    double compressedBits = static_cast<double>(totalBytes) * 8.0;
    double ratio = (compressedBits > 0) ? originalBits / compressedBits : 0.0;

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalEncodes;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);

    emit encoded(result, ratio);
    return result;
}

/** @brief 解码字节流为整数序列
 *  @param data  编码数据
 *  @param m     Golomb参数
 *  @param count 期望解码的元素数
 *  @return 解码后的整数数组 */
QVector<int> GolombCoder::decode(const QByteArray& data, int m, int count)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;
    if (data.isEmpty() || m <= 0 || count <= 0) {
        return result;
    }

    result.reserve(count);
    int bitPos = 0;

    for (int i = 0; i < count; ++i) {
        if (bitPos >= data.size() * 8) break;
        result.append(decodeSingle(data, bitPos, m));
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    ++m_stats.totalDecodes;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalEncodes + m_stats.totalDecodes);

    emit decoded(result);
    return result;
}

/** @brief 计算最优M参数(最小化编码长度)
 *  @param values 样本数据
 *  @return 最优M参数 */
int GolombCoder::optimalM(const QVector<int>& values) const
{
    if (values.isEmpty()) return 1;

    /* 计算均值 */
    double sum = 0.0;
    for (int v : values) {
        sum += qMax(0, v);
    }
    double mean = sum / static_cast<double>(values.size());

    /* 最优M: ceil(mean / ln(2)) 对Golomb编码
     * 对Rice编码(Golomb的特例，M=2^k), 取最近的2的幂 */
    if (mean <= 0.0) return 1;

    double optimalM = qCeil(mean / qLn(2.0));

    /* 取最近的2的幂(Rice编码) */
    int k = qRound(qLn(optimalM) / qLn(2.0));
    k = qBound(0, k, 20);
    int riceM = 1 << k;

    return riceM;
}

/** @brief 重置统计 */
void GolombCoder::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 编码单个值(Golomb-Rice)
 *  @param value     非负整数
 *  @param m         Golomb参数(2的幂)
 *  @param bitBuffer 位缓冲
 *  @param bitPos    位位置 */
void GolombCoder::encodeSingle(int value, int m,
                               QByteArray& bitBuffer, int& bitPos) const
{
    int k = 0;
    int tmp = m;
    while (tmp > 1) { ++k; tmp >>= 1; }

    int q = value >> k;   /* 商(value / m) */
    int r = value & (m - 1); /* 余数(value % m) */

    /* 扩展位缓冲区(按需) */
    auto ensureCapacity = [&](int bits) {
        int neededBytes = (bitPos + bits + 7) / 8;
        while (bitBuffer.size() < neededBytes) {
            bitBuffer.append('\0');
        }
    };

    /* 1. 一元编码商: q个1 + 1个0 */
    ensureCapacity(q + 1 + k);
    for (int i = 0; i < q; ++i) {
        int byteIdx = bitPos / 8;
        int bitIdx = 7 - (bitPos % 8);
        bitBuffer[byteIdx] |= static_cast<char>(1 << bitIdx);
        ++bitPos;
    }
    /* 终止0 */
    ++bitPos;

    /* 2. 二进制编码余数(k位) */
    for (int i = k - 1; i >= 0; --i) {
        if (r & (1 << i)) {
            int byteIdx = bitPos / 8;
            int bitIdx = 7 - (bitPos % 8);
            bitBuffer[byteIdx] |= static_cast<char>(1 << bitIdx);
        }
        ++bitPos;
    }
}

/** @brief 解码单个值
 *  @param data   编码数据
 *  @param bitPos 当前位位置(引用更新)
 *  @param m      Golomb参数
 *  @return 解码值 */
int GolombCoder::decodeSingle(const QByteArray& data, int& bitPos, int m) const
{
    int k = 0;
    int tmp = m;
    while (tmp > 1) { ++k; tmp >>= 1; }

    /* 1. 解码一元商: 数连续1直到遇到0 */
    int q = 0;
    while (bitPos < data.size() * 8) {
        int byteIdx = bitPos / 8;
        int bitIdx = 7 - (bitPos % 8);
        bool bit = (data[byteIdx] >> bitIdx) & 1;
        ++bitPos;
        if (!bit) break;
        ++q;
    }

    /* 2. 解码余数(k位) */
    int r = 0;
    for (int i = k - 1; i >= 0; --i) {
        if (bitPos < data.size() * 8) {
            int byteIdx = bitPos / 8;
            int bitIdx = 7 - (bitPos % 8);
            bool bit = (data[byteIdx] >> bitIdx) & 1;
            if (bit) r |= (1 << i);
        }
        ++bitPos;
    }

    return q * m + r;
}
