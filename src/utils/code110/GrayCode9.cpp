#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>
#include "GrayCode9.h"

/**
 * @brief 构造函数，初始化格雷码编解码器
 * @param parent 父对象指针
 */
GrayCode9::GrayCode9(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GrayCode9::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 设置格雷码位宽
 *
 * 位宽n决定生成序列的长度为2^n。
 * 典型值4~16，对应16~65536个码字。
 *
 * @param bits 位宽 (1~20)
 */
void GrayCode9::setBitWidth(int bits)
{
    m_bitWidth = qBound(1, bits, 20);
}

/**
 * @brief 将自然二进制数转换为格雷码
 *
 * 格雷码 = 二进制数 XOR (二进制数右移1位)。
 * 转换结果保证相邻整数只差1位。
 *
 * @param binary 自然二进制数
 * @return 对应的格雷码值
 */
int GrayCode9::binaryToGray(int binary)
{
    QElapsedTimer timer;
    timer.start();

    if (binary < 0) return 0;

    int gray = binary ^ (binary >> 1);

    m_stats.totalConverted++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConverted;

    emit conversionCompleted(1);
    return gray;
}

/**
 * @brief 将格雷码转换回自然二进制数
 *
 * 通过迭代XOR累积还原二进制值：
 * 从最高位开始，逐位将前一位的结果与当前格雷码位异或。
 *
 * @param gray 格雷码值
 * @return 对应的自然二进制数
 */
int GrayCode9::grayToBinary(int gray)
{
    QElapsedTimer timer;
    timer.start();

    if (gray < 0) return 0;

    int binary = gray;
    /* 迭代XOR累积：每次右移并异或，直到移位为0 */
    binary ^= (binary >> 1);
    binary ^= (binary >> 2);
    binary ^= (binary >> 4);
    binary ^= (binary >> 8);
    binary ^= (binary >> 16);

    m_stats.totalConverted++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConverted;

    emit conversionCompleted(1);
    return binary;
}

/**
 * @brief 生成指定位宽的完整格雷码序列
 *
 * 使用递归反射法生成格雷码序列：
 * 1. 基础情况 n=1: {0, 1}
 * 2. 递归步骤: 取n-1位的序列，正序前缀0，逆序前缀1，拼接
 *
 * @return 格雷码序列 (长度 = 2^m_bitWidth)
 */
QVector<int> GrayCode9::generateSequence()
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> sequence;

    if (m_bitWidth < 1 || m_bitWidth > 20) {
        emit conversionCompleted(0);
        return sequence;
    }

    int totalCodes = 1 << m_bitWidth;
    sequence.reserve(totalCodes);

    /* 迭代反射法构造格雷码序列 */
    /* 初始: 1位格雷码 {0, 1} */
    sequence.append(0);
    sequence.append(1);

    for (int bit = 2; bit <= m_bitWidth; ++bit) {
        int currentSize = sequence.size();
        /* 逆序遍历，前缀1（即加上2^(bit-1)） */
        for (int i = currentSize - 1; i >= 0; --i) {
            sequence.append(sequence[i] | (1 << (bit - 1)));
        }
    }

    m_stats.totalConverted += sequence.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConverted;

    emit conversionCompleted(sequence.size());
    return sequence;
}
