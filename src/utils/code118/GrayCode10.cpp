#include "GrayCode10.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化格雷码转换引擎
 * @param parent 父对象指针
 */
GrayCode10::GrayCode10(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 重置所有统计信息
 */
void GrayCode10::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 生成指定位宽的格雷码序列
 *
 * 使用镜像前缀法生成n位格雷码序列。
 * 前半部分为0+上一级序列，后半部分为1+上一级序列反转。
 * 结果序列中相邻两个格雷码恰好有一位不同。
 *
 * @param numBits 位宽
 * @return 格雷码整数序列
 */
QVector<int> GrayCode10::generateSequence(int numBits)
{
    QElapsedTimer timer;
    timer.start();

    if (numBits <= 0) {
        emit conversionCompleted(0);
        return {};
    }

    /* 镜像前缀法 */
    QVector<int> sequence;
    int total = 1 << numBits;
    sequence.reserve(total);

    for (int i = 0; i < total; ++i) {
        sequence.append(binaryToGray(i));
    }

    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalConvertOps++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalConvertOps;

    emit conversionCompleted(sequence.size());
    return sequence;
}

/**
 * @brief 二进制转格雷码
 *
 * 公式：gray = n XOR (n >> 1)
 * 将自然二进制数的最高位保持不变，其余各位与上一位异或。
 *
 * @param binary 输入二进制值
 * @return 对应格雷码值
 */
int GrayCode10::binaryToGray(int binary) const
{
    return binary ^ (binary >> 1);
}

/**
 * @brief 格雷码转二进制
 *
 * 通过迭代异或累积还原二进制值：
 * 从最高位开始，每位与之前累积结果异或。
 *
 * @param gray 输入格雷码值
 * @return 对应二进制值
 */
int GrayCode10::grayToBinary(int gray) const
{
    int binary = gray;
    binary ^= (binary >> 1);
    binary ^= (binary >> 2);
    binary ^= (binary >> 4);
    binary ^= (binary >> 8);
    binary ^= (binary >> 16);
    return binary;
}

/**
 * @brief 计算两个格雷码之间的汉明距离
 *
 * 先将两个格雷码转回二进制，再计算二进制之间的
 * 汉明距离（不同位的个数）。
 *
 * @param grayA 格雷码A
 * @param grayB 格雷码B
 * @return 汉明距离
 */
int GrayCode10::hammingDistance(int grayA, int grayB) const
{
    int xorVal = grayToBinary(grayA) ^ grayToBinary(grayB);
    int dist = 0;
    while (xorVal != 0) {
        dist += xorVal & 1;
        xorVal >>= 1;
    }
    return dist;
}
