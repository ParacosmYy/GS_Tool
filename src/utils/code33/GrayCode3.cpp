/**
 * @file GrayCode3.cpp
 * @brief 格雷码增强实现 — 编解码/汉明距离矩阵/邻居序列生成
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 格雷码(Gray Code)是一种二进制编码方式，使得相邻两个码字之间
 * 恰好只有一位不同。这一特性使其在以下场景中有广泛应用:
 * - 旋转编码器: 减少状态转换时的毛刺
 * - 模数转换器: 减少量化误差
 * - 通信系统: 降低误码率
 * - 组合优化: 格码空间中的邻域搜索
 */

#include "utils/code33/GrayCode3.h"

#include <QElapsedTimer>

#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
GrayCode3::GrayCode3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("GrayCode3"));
}

/**
 * @brief 二进制转格雷码
 *
 * 公式: gray = binary XOR (binary >> 1)
 * 格雷码中相邻两个码字之间恰好有一位不同。
 *
 * @param binary 二进制值
 * @return 格雷码值
 */
quint32 GrayCode3::encode(quint32 binary) const
{
    return binary ^ (binary >> 1);
}

/**
 * @brief 格雷码转二进制
 *
 * 逐位异或还原: 从最高位开始，每个bit = gray_bit XOR 前一个还原的bit。
 *
 * @param gray 格雷码值
 * @return 二进制值
 */
quint32 GrayCode3::decode(quint32 gray) const
{
    quint32 binary = gray;
    binary ^= (binary >> 1);
    binary ^= (binary >> 2);
    binary ^= (binary >> 4);
    binary ^= (binary >> 8);
    binary ^= (binary >> 16);
    return binary;
}

/**
 * @brief 生成n位格雷码的完整序列
 *
 * 使用镜像法: n位格雷码序列 = (n-1)位序列前缀0 + (n-1)位逆序前缀1。
 * 序列长度为 2^bits，相邻元素恰好一位不同。
 *
 * @param bits 位宽（1~31）
 * @return 格雷码序列
 */
QVector<quint32> GrayCode3::generateSequence(int bits)
{
    QElapsedTimer timer;
    timer.start();

    bits = qBound(1, bits, 31);
    int size = 1 << bits;
    QVector<quint32> seq;
    seq.reserve(size);

    /* 基础: 1位格雷码 */
    seq.append(0);
    seq.append(1);

    /* 逐位扩展: 镜像+前缀 */
    for (int b = 2; b <= bits; ++b) {
        int curSize = seq.size();
        /* 逆序遍历，添加前缀1（即设置第b-1位） */
        for (int i = curSize - 1; i >= 0; --i) {
            seq.append(seq[i] | (1u << (b - 1)));
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalEncodes += size;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalEncodes + m_stats.totalDecodes);

    emit sequenceGenerated(bits, size);
    return seq;
}

/**
 * @brief 计算两个值的汉明距离
 *
 * 统计a XOR b结果中置1的位数。
 *
 * @param a 第一个值
 * @param b 第二个值
 * @return 汉明距离（不同位数）
 */
int GrayCode3::hammingDistance(quint32 a, quint32 b) const
{
    quint32 x = a ^ b;
    /* Brian Kernighan位计数 */
    int count = 0;
    while (x) {
        x &= (x - 1);
        count++;
    }
    return count;
}

/**
 * @brief 生成n位格雷码的距离矩阵
 *
 * 距离矩阵中元素 [i][j] = hammingDistance(encode(i), encode(j))。
 * 用于分析格雷码的误差检测/纠正特性。
 *
 * @param bits 位宽
 * @return 距离矩阵 [2^bits x 2^bits]
 */
QVector<QVector<int>> GrayCode3::distanceMatrix(int bits)
{
    QElapsedTimer timer;
    timer.start();

    bits = qBound(1, bits, 12); /* 限制避免内存爆炸 */
    int size = 1 << bits;

    QVector<QVector<int>> mat(size, QVector<int>(size, 0));

    /* 预计算所有格雷码 */
    QVector<quint32> grayCodes(size);
    for (int i = 0; i < size; ++i) {
        grayCodes[i] = encode(static_cast<quint32>(i));
    }

    /* 计算距离矩阵（对称，只需计算上三角） */
    for (int i = 0; i < size; ++i) {
        mat[i][i] = 0;
        for (int j = i + 1; j < size; ++j) {
            int d = hammingDistance(grayCodes[i], grayCodes[j]);
            mat[i][j] = d;
            mat[j][i] = d;
        }
    }

    m_timeSum += timer.elapsed();
    m_stats.totalEncodes += size;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, m_stats.totalEncodes + m_stats.totalDecodes);

    return mat;
}

/**
 * @brief 获取给定格雷码的所有单比特翻转邻居
 *
 * 返回所有与输入格雷码恰好差1位的格雷码。
 * 在n位格码空间中，每个码字恰好有n个邻居，对应翻转不同的位。
 * 用于错误检测和格码空间中的路径搜索。
 *
 * @param gray 输入格雷码
 * @param bits 位宽
 * @return 邻居格雷码列表（bits个）
 */
QVector<quint32> GrayCode3::neighbors(quint32 gray, int bits)
{
    bits = qBound(1, bits, 31);
    QVector<quint32> result;
    result.reserve(bits);

    for (int i = 0; i < bits; ++i) {
        result.append(gray ^ (1u << i));
    }

    return result;
}

/**
 * @brief 检查一个序列是否为合法的格雷码序列
 *
 * 验证条件:
 * 1. 序列中所有元素互不相同
 * 2. 相邻元素的汉明距离均为1
 * 3. 首尾元素的汉明距离也为1（循环格雷码）
 *
 * @param sequence 待验证的格雷码序列
 * @param bits 位宽
 * @return 是否为合法格雷码序列
 */
bool GrayCode3::isValidSequence(const QVector<quint32>& sequence, int bits) const
{
    if (sequence.isEmpty() || bits < 1) return false;
    int expectedSize = 1 << bits;

    /* 检查长度 */
    if (sequence.size() != expectedSize) return false;

    /* 检查所有值在有效范围内 */
    quint32 mask = (1u << bits) - 1u;
    QSet<quint32> seen;
    for (quint32 val : sequence) {
        if ((val & ~mask) != 0) return false;
        if (seen.contains(val)) return false;
        seen.insert(val);
    }

    /* 检查相邻元素汉明距离为1 */
    for (int i = 0; i < sequence.size(); ++i) {
        int next = (i + 1) % sequence.size();
        int dist = hammingDistance(sequence[i], sequence[next]);
        if (dist != 1) return false;
    }

    return true;
}

/**
 * @brief 将格雷码转换为二进制字符串表示
 *
 * 输出指定位宽的0/1字符串，方便调试和可视化。
 *
 * @param gray 格雷码值
 * @param bits 显示位宽
 * @return 二进制字符串
 */
QString GrayCode3::toString(quint32 gray, int bits) const
{
    bits = qBound(1, bits, 31);
    QString result;
    for (int i = bits - 1; i >= 0; --i) {
        result.append((gray & (1u << i)) ? QLatin1Char('1') : QLatin1Char('0'));
    }
    return result;
}

/**
 * @brief 重置所有累积统计信息
 */
void GrayCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
