/**
 * @file GrayCode5.cpp
 * @brief 格雷码编解码器实现（第5版）
 *
 * 实现二进制格雷码的编码和解码。格雷码是一种相邻两数
 * 只有一个比特不同的编码方式，常用于减少状态转换错误。
 * 支持批量编码/解码、单值操作、汉明距离计算和完整码表生成。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code65/GrayCode5.h"

#include <QElapsedTimer>
#include <QtMath>

/**
 * @brief 构造函数，初始化格雷码处理器
 * @param parent 父QObject对象指针
 */
GrayCode5::GrayCode5(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置位宽
 * @param bits 每个值的比特位数（1~31）
 */
void GrayCode5::setBitWidth(int bits)
{
    m_bits = qBound(1, bits, 31);
}

/**
 * @brief 编码单个值为格雷码
 * @param value 输入的自然二进制值
 * @return 对应的格雷码值
 */
int GrayCode5::encodeSingle(int value) const
{
    return value ^ (value >> 1);
}

/**
 * @brief 解码单个格雷码值为自然二进制
 * @param gray 输入的格雷码值
 * @return 解码后的自然二进制值
 */
int GrayCode5::decodeSingle(int gray) const
{
    int value = 0;
    while (gray) {
        value ^= gray;
        gray >>= 1;
    }
    return value;
}

/**
 * @brief 计算两个值之间的汉明距离
 * @param a 第一个值
 * @param b 第二个值
 * @return 不同的比特位数
 */
int GrayCode5::hammingDistance(int a, int b) const
{
    int xorVal = a ^ b;
    int dist = 0;
    while (xorVal) {
        dist += xorVal & 1;
        xorVal >>= 1;
    }
    return dist;
}

/**
 * @brief 生成完整的格雷码序列
 *
 * 使用镜像反射法生成n位格雷码的完整序列。
 * 序列长度为2^bits，相邻两数只有一个比特不同。
 *
 * @return 格雷码序列（按自然顺序对应的格雷码值）
 */
QVector<int> GrayCode5::generateCode() const
{
    int total = 1 << m_bits;
    QVector<int> code;
    code.reserve(total);

    for (int i = 0; i < total; ++i) {
        code.append(encodeSingle(i));
    }

    return code;
}

/**
 * @brief 批量编码数据序列为格雷码
 *
 * 将输入的自然二进制序列逐值转换为格雷码。
 * 输出长度与输入相同。
 *
 * @param data 输入的自然二进制数据序列
 * @return 格雷码编码后的序列
 */
QVector<int> GrayCode5::encode(const QVector<int>& data)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> encoded;
    if (data.isEmpty()) {
        emit encodeCompleted(0, m_bits);
        return encoded;
    }

    int n = data.size();
    encoded.reserve(n);
    int mask = (1 << m_bits) - 1;

    for (int i = 0; i < n; ++i) {
        int val = data[i] & mask;
        encoded.append(encodeSingle(val));
    }

    /* 更新统计 */
    m_stats.totalEncodes++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(n, m_bits);
    return encoded;
}

/**
 * @brief 批量解码格雷码序列为自然二进制
 *
 * 将输入的格雷码序列逐值转换回自然二进制。
 *
 * @param gray 输入的格雷码序列
 * @return 解码后的自然二进制序列
 */
QVector<int> GrayCode5::decode(const QVector<int>& gray)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    if (gray.isEmpty()) return decoded;

    int n = gray.size();
    decoded.reserve(n);

    for (int i = 0; i < n; ++i) {
        decoded.append(decodeSingle(gray[i]));
    }

    /* 更新统计 */
    m_stats.totalDecodes++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncodes + m_stats.totalDecodes);

    return decoded;
}

/**
 * @brief 获取当前统计信息
 * @return 编解码统计结构
 */
GrayCode5::Stats GrayCode5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void GrayCode5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 验证格雷码序列的正确性
 *
 * 检查生成的格雷码序列是否满足格雷码性质：
 * 相邻两数之间只有一个比特不同。
 *
 * @return 序列是否满足格雷码性质
 */
bool GrayCode5::validateSequence() const
{
    QVector<int> code = generateCode();
    for (int i = 1; i < code.size(); ++i) {
        if (hammingDistance(code[i], code[i - 1]) != 1) {
            return false;
        }
    }
    /* 首尾也应只有一个比特不同（循环格雷码） */
    if (code.size() > 2) {
        if (hammingDistance(code.first(), code.last()) != 1) {
            return false;
        }
    }
    return true;
}

/**
 * @brief 计算格雷码的总转换次数
 *
 * 在完整格雷码序列中，每个比特位发生翻转的次数。
 * 可用于分析格雷码的均衡性。
 *
 * @return 每个比特位的翻转次数
 */
QVector<int> GrayCode5::transitionCounts() const
{
    QVector<int> code = generateCode();
    QVector<int> counts(m_bits, 0);

    for (int i = 1; i < code.size(); ++i) {
        int diff = code[i] ^ code[i - 1];
        for (int b = 0; b < m_bits; ++b) {
            if (diff & (1 << b)) {
                counts[b]++;
            }
        }
    }

    return counts;
}
