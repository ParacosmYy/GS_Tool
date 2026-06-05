#include "GrayCode8.h"
#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/**
 * @file GrayCode8.cpp
 * @brief Gray码编解码器实现
 *
 * Gray码(格雷码)的特性: 相邻两个码字之间恰好有一位不同。
 * 二进制转Gray码: G = B ^ (B >> 1)
 * Gray码转二进制: 依次异或已确定的高位
 */

/**
 * @brief 构造函数，初始化默认位长度
 * @param parent 父QObject对象指针
 */
GrayCode8::GrayCode8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置位长度
 * @param bits Gray码的位数(1~31)
 */
void GrayCode8::setBitLength(int bits)
{
    m_bitLength = qBound(1, bits, 31);
}

/**
 * @brief 将整数编码为Gray码
 *
 * 转换公式: gray = binary ^ (binary >> 1)
 * 例如: 3(011) -> 2(010), 5(101) -> 7(111)
 *
 * @param value 输入整数值(二进制)
 * @return Gray码值
 */
int GrayCode8::encode(int value)
{
    QElapsedTimer timer;
    timer.start();

    const int gray = value ^ (value >> 1);

    m_stats.totalCoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCoded;

    return gray;
}

/**
 * @brief 将Gray码解码为整数
 *
 * 逐位恢复: 从最高位开始，每一位都是Gray码位与已恢复的高位异或。
 * B[n-1] = G[n-1]
 * B[i] = G[i] ^ B[i+1]
 *
 * @param grayValue 输入Gray码值
 * @return 解码后的整数值(二进制)
 */
int GrayCode8::decode(int grayValue)
{
    QElapsedTimer timer;
    timer.start();

    int binary = grayValue;
    int mask = grayValue >> 1;

    while (mask != 0) {
        binary ^= mask;
        mask >>= 1;
    }

    m_stats.totalCoded++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCoded;

    return binary;
}

/**
 * @brief 生成所有Gray码序列
 *
 * 使用镜像法生成完整的n位Gray码序列:
 * 1. 1位Gray码: [0, 1]
 * 2. n位Gray码 = [0+(n-1位序列)] + [1+(n-1位序列的镜像)]
 *
 * @return 所有可能的Gray码值(按序列顺序)
 */
QVector<int> GrayCode8::generateAll()
{
    QElapsedTimer timer;
    timer.start();

    const int n = m_bitLength;
    const int total = 1 << n;
    QVector<int> codes;
    codes.reserve(total);

    for (int i = 0; i < total; ++i) {
        codes.append(i ^ (i >> 1));
    }

    m_stats.totalCoded += total;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalCoded;

    emit generated(total);
    return codes;
}

/**
 * @brief 重置所有统计信息
 */
void GrayCode8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
