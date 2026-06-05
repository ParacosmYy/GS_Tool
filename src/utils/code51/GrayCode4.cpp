/**
 * @file GrayCode4.cpp
 * @brief 格雷码编解码器实现
 *
 * 实现格雷码（Gray Code / 反射二进制码）的编码、解码和序列生成。
 * 格雷码的核心特性是相邻两个码字之间恰好有一位不同（汉明距离为1），
 * 这使得它在以下领域有广泛应用:
 *
 * 1. 旋转编码器: 避免位置转换时的多位同时跳变导致读数错误
 * 2. 模数转换器（ADC）: 减少量化噪声和亚稳态问题
 * 3. 数字通信: 降低误码率，简化差错检测
 * 4. 组合逻辑优化: 卡诺图（Karnaugh Map）的基础编码
 * 5. 遗传算法: 减少变异操作对适应度的剧烈影响
 *
 * 编码公式: gray = binary ^ (binary >> 1)
 * 解码公式: 逐位异或恢复（从高位到低位）
 *
 * 示例（4位格雷码）:
 * 十进制 | 二进制 | 格雷码
 *   0    |  0000  |  0000
 *   1    |  0001  |  0001
 *   2    |  0010  |  0011
 *   3    |  0011  |  0010
 *   4    |  0100  |  0110
 *   5    |  0101  |  0111
 *   6    |  0110  |  0101
 *   7    |  0111  |  0100
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/code51/GrayCode4.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化默认位数为8
 *
 * 默认8位格雷码可表示0~255共256个值，适用于大多数旋转编码器场景。
 * @param parent 父QObject对象指针
 */
GrayCode4::GrayCode4(QObject* parent)
    : QObject(parent)
    , m_bits(8)
    , m_timeSum(0.0)
{
}

/**
 * @brief 设置格雷码位数
 *
 * 位数决定了可表示的数值范围: 0 ~ 2^bits - 1。
 * 例如: 4位可表示0~15，8位可表示0~255，16位可表示0~65535。
 *
 * @param bits 位数，范围 [1, 31]，限制为31位以避免int溢出
 */
void GrayCode4::setBits(int bits)
{
    m_bits = qBound(1, bits, 31);
}

/**
 * @brief 将二进制值编码为格雷码
 *
 * 使用异或运算实现: gray = value ^ (value >> 1)
 * 这是格雷码编码的标准公式，将二进制数转换为反射二进制码。
 *
 * 编码原理: 将二进制数的每一位与其左边邻位异或。
 * 右移1位后异或等价于: gray[i] = binary[i] XOR binary[i+1]
 * 最高位保持不变: gray[msb] = binary[msb]
 *
 * @param value 待编码的二进制值，范围 [0, 2^bits - 1]
 * @return 对应的格雷码值
 */
int GrayCode4::encode(int value) const
{
    QElapsedTimer timer;
    timer.start();

    /* 掩码截断到有效位数，防止超出范围的位干扰结果 */
    int mask = (1 << m_bits) - 1;
    int truncated = value & mask;

    /* 格雷码编码公式: gray = binary XOR (binary >> 1) */
    int gray = truncated ^ (truncated >> 1);

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    const_cast<GrayCode4*>(this)->m_stats.totalEncodes++;
    const_cast<GrayCode4*>(this)->m_timeSum += elapsed;
    int total = m_stats.totalEncodes + m_stats.totalDecodes;
    const_cast<GrayCode4*>(this)->m_stats.avgProcessingTimeMs =
        (total > 0) ? m_timeSum / total : 0.0;

    const_cast<GrayCode4*>(this)->emit encodeCompleted(value, gray);
    return gray;
}

/**
 * @brief 将格雷码解码为二进制值
 *
 * 逐位异或恢复原二进制值。通过连续右移异或实现:
 * binary = gray ^ (gray>>1) ^ (gray>>2) ^ ... ^ (gray>>(bits-1))
 *
 * 解码原理: gray[i] = binary[i] XOR binary[i+1]
 * 因此: binary[i] = gray[i] XOR binary[i+1]
 * 从最高位开始（最高位不变），逐位向低序位推导。
 *
 * 等价的高效实现: 将gray连续右移1,2,4,...位并异或到结果中。
 *
 * @param gray 待解码的格雷码值
 * @return 对应的二进制值
 */
int GrayCode4::decode(int gray) const
{
    QElapsedTimer timer;
    timer.start();

    /* 掩码截断到有效位数 */
    int mask = (1 << m_bits) - 1;
    gray = gray & mask;

    /* 连续右移异或解码，等价于从高位到低位逐位恢复 */
    int binary = gray;
    int shift = gray >> 1;

    /* 迭代直到所有有效位都处理完毕 */
    while (shift != 0) {
        binary ^= shift;
        shift >>= 1;
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    const_cast<GrayCode4*>(this)->m_stats.totalDecodes++;
    const_cast<GrayCode4*>(this)->m_timeSum += elapsed;
    int total = m_stats.totalEncodes + m_stats.totalDecodes;
    const_cast<GrayCode4*>(this)->m_stats.avgProcessingTimeMs =
        (total > 0) ? m_timeSum / total : 0.0;

    return binary;
}

/**
 * @brief 生成完整的格雷码序列
 *
 * 生成 2^bits 个格雷码值，按自然顺序排列。
 * 序列中相邻两个值之间恰好有一位不同（汉明距离为1），
 * 这是格雷码的反射特性决定的。
 *
 * 反射法生成原理（以3位为例）:
 * 1. 1位格雷码: [0, 1]
 * 2. 镜像追加: [0, 1, 1, 0]
 * 3. 前半加前缀0: [00, 01, 11, 10]
 * 即 i ^ (i >> 1) 的自然序结果。
 *
 * @return 格雷码序列，长度为 2^bits
 */
QVector<int> GrayCode4::generateSequence() const
{
    QElapsedTimer timer;
    timer.start();

    int count = 1 << m_bits;
    QVector<int> sequence;
    sequence.reserve(count);

    /* 使用编码公式直接生成每个位置的格雷码值 */
    for (int i = 0; i < count; ++i) {
        sequence.append(i ^ (i >> 1));
    }

    /* 累加计时 */
    double elapsed = timer.elapsed();
    const_cast<GrayCode4*>(this)->m_timeSum += elapsed;

    return sequence;
}

/**
 * @brief 计算两个值的汉明距离
 *
 * 汉明距离为两个等长二进制串之间不同位的个数。
 * 使用Brian Kernighan算法高效计算: 每次迭代清除最低位的1。
 *
 * 算法步骤:
 * 1. 计算异或值 xorVal = a ^ b
 * 2. 每次执行 xorVal &= (xorVal - 1) 清除最低位的1
 * 3. 计数执行次数即为1的位数
 *
 * 时间复杂度: O(popcount(a^b))，最坏情况 O(bits)
 *
 * @param a 第一个值
 * @param b 第二个值
 * @return 汉明距离，即不同位的数量
 */
int GrayCode4::hammingDistance(int a, int b) const
{
    /* 计算异或结果中1的位数 */
    int xorVal = a ^ b;
    int distance = 0;

    /* Brian Kernighan算法: 每次清除最低位的1 */
    while (xorVal != 0) {
        xorVal &= (xorVal - 1);
        distance++;
    }

    return distance;
}

/**
 * @brief 重置所有统计计数器
 *
 * 将编码次数、解码次数、平均处理时间等统计指标归零。
 * 不影响位数设置。
 */
void GrayCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
