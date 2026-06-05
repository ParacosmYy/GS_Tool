#include "GrayCode6.h"
#include <QElapsedTimer>
#include <QtMath>

/**
 * @class GrayCode6
 * @brief Gray码生成器与转换器实现
 *
 * Gray码(格雷码)是一种相邻两个码字之间只有一位不同的二进制编码。
 * 广泛用于旋转编码器、Karnaugh图、模数转换等场景，可有效减少
 * 状态切换时的毛刺和错误。
 */

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
GrayCode6::GrayCode6(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 生成指定位数的Gray码序列
 *
 * 使用镜像反射法生成完整的n位Gray码序列。
 * 算法: 从1位Gray码[0,1]开始，每次镜像扩展并给上半部分最高位加1。
 * 时间复杂度 O(2^n)。
 *
 * @param bits Gray码位数(1~31)
 * @return 包含2^bits个Gray码值的向量
 */
QVector<int> GrayCode6::generate(int bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> result;

    if (bits < 1 || bits > 31) {
        m_timeSum += timer.elapsed();
        return result;
    }

    /* 初始化1位Gray码 */
    result.append(0);
    result.append(1);

    /* 逐位扩展 */
    for (int i = 2; i <= bits; ++i) {
        int size = result.size();
        /* 从后向前镜像复制 */
        for (int j = size - 1; j >= 0; --j) {
            result.append(result[j] | (1 << (i - 1)));
        }
    }

    m_stats.totalSequencesGenerated++;
    m_stats.totalConversions += result.size();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalSequencesGenerated);

    emit sequenceGenerated(bits, result.size());

    return result;
}

/**
 * @brief 二进制数转换为Gray码
 *
 * 公式: gray = binary XOR (binary >> 1)
 * 二进制最高位保持不变，其余每位等于原码对应位与高一位的异或。
 *
 * @param binary 输入的二进制数值
 * @return 对应的Gray码值
 */
int GrayCode6::binaryToGray(int binary) const
{
    return binary ^ (binary >> 1);
}

/**
 * @brief Gray码转换为二进制数
 *
 * 从最高位开始逐位恢复: 每位等于Gray码对应位与已恢复的高一位的异或。
 * 公式: binary[i] = gray[i] XOR binary[i+1]
 *
 * @param gray 输入的Gray码值
 * @return 对应的二进制数值
 */
int GrayCode6::grayToBinary(int gray) const
{
    int binary = gray;
    int mask = gray;
    while (mask) {
        mask >>= 1;
        binary ^= mask;
    }
    return binary;
}

/**
 * @brief 生成n位Gray码的所有相邻差分
 *
 * 计算Gray码序列中相邻码字之间的汉明距离(始终为1)，
 * 并返回每个位置的翻转位索引。可用于解码和错误检测。
 *
 * @param bits Gray码位数
 * @return 每个位置翻转的位索引(0-indexed)
 */
QVector<int> GrayCode6::generateFlipSequence(int bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> flips;

    if (bits < 1 || bits > 31) {
        m_timeSum += timer.elapsed();
        return flips;
    }

    /* 生成完整Gray码序列 */
    QVector<int> seq = generate(bits);

    /* 计算相邻码字的异或，找出翻转位 */
    for (int i = 1; i < seq.size(); ++i) {
        int diff = seq[i] ^ seq[i - 1];
        int bitPos = 0;
        while (diff > 1) {
            diff >>= 1;
            bitPos++;
        }
        flips.append(bitPos);
    }

    m_timeSum += timer.elapsed();

    return flips;
}

/**
 * @brief 验证序列是否为合法的Gray码
 *
 * 检查序列中每对相邻元素是否恰好只有一位不同。
 *
 * @param sequence 待验证的码字序列
 * @return true如果是合法的Gray码序列
 */
bool GrayCode6::validateGraySequence(const QVector<int>& sequence) const
{
    if (sequence.size() < 2) return true;

    for (int i = 1; i < sequence.size(); ++i) {
        int diff = sequence[i] ^ sequence[i - 1];
        /* 恰好一位不同 => diff是2的幂 */
        if (diff == 0 || (diff & (diff - 1)) != 0) {
            return false;
        }
    }

    return true;
}

/**
 * @brief 重置所有统计数据
 *
 * 将序列生成计数、转换计数和计时归零。
 */
void GrayCode6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
