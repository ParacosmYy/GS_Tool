#include "GrayCode7.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @brief 构造函数，初始化Gray码编解码器
 * @param parent 父对象指针
 */
GrayCode7::GrayCode7(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置编码位长度
 * @param bits 编码的比特位数
 */
void GrayCode7::setBitLength(int bits)
{
    m_bitLength = qBound(1, bits, 32);
}

/**
 * @brief 将整数编码为Gray码
 *
 * Gray码编码公式: gray = n ^ (n >> 1)
 * 相邻两个编码之间恰好有一位不同。
 *
 * @param value 待编码的整数值
 */
void GrayCode7::encode(int value)
{
    QElapsedTimer timer;
    timer.start();

    int grayValue = value ^ (value >> 1);
    Q_UNUSED(grayValue)

    m_timeSum += timer.elapsed();
    m_stats.totalGenerated++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerated;
    emit generated(1);
}

/**
 * @brief 将Gray码解码为整数
 *
 * Gray码解码通过逐位异或恢复原始值：
 * mask从高位向低位扫描，每次将当前位与高位异或。
 *
 * @param grayValue Gray码值
 */
void GrayCode7::decode(int grayValue)
{
    QElapsedTimer timer;
    timer.start();

    int value = grayValue;
    for (int mask = value >> 1; mask != 0; mask >>= 1) {
        value ^= mask;
    }
    Q_UNUSED(value)

    m_timeSum += timer.elapsed();
    m_stats.totalGenerated++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerated;
    emit generated(1);
}

/**
 * @brief 生成指定位长的全部Gray码序列
 *
 * 使用反射法递归生成n位Gray码：
 * 1. 1位Gray码: [0, 1]
 * 2. n位Gray码 = [0+G(n-1), 1+reverse(G(n-1))]
 *
 * 也可以直接用公式 gray = i ^ (i >> 1) 顺序生成。
 */
void GrayCode7::generateAll()
{
    QElapsedTimer timer;
    timer.start();

    int totalCodes = 1 << m_bitLength;

    /* 使用公式法生成全部Gray码 */
    for (int i = 0; i < totalCodes; ++i) {
        int grayCode = i ^ (i >> 1);
        Q_UNUSED(grayCode)
    }

    m_timeSum += timer.elapsed();
    m_stats.totalGenerated++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalGenerated;
    emit generated(totalCodes);
}

/**
 * @brief 将Gray码序列转换为二进制序列
 * @param graySequence Gray码值列表
 * @return 对应的二进制值列表
 */
QVector<int> GrayCode7::grayToBinary(const QVector<int>& graySequence) const
{
    QVector<int> result;
    for (int gray : graySequence) {
        int value = gray;
        for (int mask = value >> 1; mask != 0; mask >>= 1) {
            value ^= mask;
        }
        result.append(value);
    }
    return result;
}

/**
 * @brief 将二进制序列转换为Gray码序列
 * @param binarySequence 二进制值列表
 * @return 对应的Gray码值列表
 */
QVector<int> GrayCode7::binaryToGray(const QVector<int>& binarySequence) const
{
    QVector<int> result;
    for (int val : binarySequence) {
        result.append(val ^ (val >> 1));
    }
    return result;
}

/**
 * @brief 计算两个相邻Gray码的汉明距离
 * @return 汉明距离(理论上始终为1)
 */
int GrayCode7::adjacentHammingDistance() const
{
    return 1;
}

/**
 * @brief 重置统计数据
 */
void GrayCode7::resetStatistics()
{
    m_stats.totalGenerated = 0;
    m_stats.avgProcessingTimeMs = 0.0;
    m_timeSum = 0.0;
}
