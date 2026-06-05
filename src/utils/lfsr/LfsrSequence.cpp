/**
 * @file LfsrSequence.cpp
 * @brief LFSR线性反馈移位寄存器实现
 */

#include "utils/lfsr/LfsrSequence.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
LfsrSequence::LfsrSequence(QObject* parent)
    : QObject(parent)
{
    /* 默认: x^8 + x^6 + x^5 + x^4 + 1 (maximal-length) */
    setPolynomial({4, 5, 6}, 8);
}

/**
 * @brief 设置反馈多项式
 * @param taps 反馈抽头位置列表(从1开始)
 * @param degree 寄存器位宽
 */
void LfsrSequence::setPolynomial(const QVector<int>& taps, int degree)
{
    m_degree = qBound(1, degree, static_cast<int>(MAX_DEGREE));
    m_taps = taps;

    /* 预计算抽头掩码 */
    m_tapMask = 0;
    for (int t : m_taps) {
        if (t > 0 && t <= m_degree) {
            m_tapMask |= (1U << (t - 1));
        }
    }

    /* 确保状态非零 */
    if (m_state == 0) m_state = 1;
}

/**
 * @brief 生成指定比特数的序列(打包为字节)
 * @param bits 所需比特数
 * @return 生成的字节序列(MSB优先)
 */
QByteArray LfsrSequence::generate(int bits)
{
    if (bits <= 0 || m_degree <= 0) return {};

    m_timer.start();

    int bytes = (bits + 7) / 8;
    QByteArray result(bytes, 0);

    for (int i = 0; i < bits; ++i) {
        int bit = step();
        int byteIdx = i / 8;
        int bitIdx = 7 - (i % 8);  /* MSB优先 */
        if (bit) {
            result[byteIdx] |= static_cast<char>(1 << bitIdx);
        }
    }

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalGenerated;
    m_stats.totalBits += static_cast<quint64>(bits);
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalGenerated);

    emit generated(bits);
    return result;
}

/**
 * @brief 生成指定数量的比特
 * @param count 比特数量
 * @return 比特值列表(0或1)
 */
QVector<int> LfsrSequence::generateBits(int count)
{
    if (count <= 0 || m_degree <= 0) return {};

    m_timer.start();

    QVector<int> bits;
    bits.reserve(count);

    for (int i = 0; i < count; ++i) {
        bits.append(step());
    }

    /* 更新统计 */
    double elapsed = m_timer.elapsed();
    m_totalTimeMs += elapsed;
    ++m_stats.totalGenerated;
    m_stats.totalBits += static_cast<quint64>(count);
    m_stats.avgProcessingTimeMs = m_totalTimeMs
        / static_cast<double>(m_stats.totalGenerated);

    emit generated(count);
    return bits;
}

/**
 * @brief 检查当前多项式是否产生最大长度序列
 * m序列的周期 = 2^degree - 1
 * @return true表示m序列
 */
bool LfsrSequence::maximalLength() const
{
    int period = detectPeriod();
    int expected = (1 << m_degree) - 1;
    return period == expected;
}

/** @brief 重置统计 */
void LfsrSequence::resetStatistics()
{
    m_stats = Stats{};
    m_totalTimeMs = 0.0;
}

/**
 * @brief 执行单步移位并返回输出比特
 * Fibonacci LFSR: 抽头位异或 -> 输入到最低位
 * @return 输出比特(0或1)
 */
int LfsrSequence::step()
{
    /* 输出最高有效位 */
    int outputBit = (m_state >> (m_degree - 1)) & 1;

    /* 计算反馈 = 抽头位的异或 */
    quint32 tapped = m_state & m_tapMask;
    int feedback = 0;
    while (tapped) {
        feedback ^= (tapped & 1);
        tapped >>= 1;
    }

    /* 移位: 高位丢弃，低位补反馈 */
    m_state = (m_state >> 1) | (static_cast<quint32>(feedback) << (m_degree - 1));

    /* 防止全零状态 */
    if (m_state == 0) {
        m_state = 1;
    }

    return outputBit;
}

/**
 * @brief 检测序列周期
 * 从当前状态出发，计数回到初始状态所需的步数
 * @return 周期长度
 */
int LfsrSequence::detectPeriod() const
{
    /* 复制状态，不修改原始对象 */
    quint32 savedState = m_state;
    quint32 startState = savedState;

    /* 临时修改: 需要临时去掉const来操作状态 */
    quint32 tempState = startState;
    int period = 0;
    int maxSteps = (1 << m_degree);

    /* 临时计算: 直接在局部模拟LFSR */
    for (int i = 0; i < maxSteps; ++i) {
        /* 一步LFSR */
        quint32 tapped = tempState & m_tapMask;
        int feedback = 0;
        while (tapped) {
            feedback ^= (tapped & 1);
            tapped >>= 1;
        }
        tempState = (tempState >> 1)
            | (static_cast<quint32>(feedback) << (m_degree - 1));
        if (tempState == 0) tempState = 1;

        ++period;
        if (tempState == startState) {
            return period;
        }
    }

    /* 未找到周期(超过2^degree步) */
    return period;
}
