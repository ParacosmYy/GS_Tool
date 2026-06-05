/**
 * @file ConvolutionalCode3.cpp
 * @brief 卷积码编解码器实现（第3版）
 *
 * 实现卷积码编码和Viterbi解码算法。支持自定义生成多项式、
 * 约束长度配置，以及可选的网格终止。Viterbi解码使用软判决输入。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code63/ConvolutionalCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/**
 * @brief 构造函数，初始化卷积码编解码器
 * @param parent 父QObject对象指针
 */
ConvolutionalCode3::ConvolutionalCode3(QObject* parent)
    : QObject(parent)
{
    /* 默认生成多项式：G1=0171(八进制), G2=0133(八进制) */
    m_gens = {0171, 0133};
}

/**
 * @brief 设置生成多项式集合
 * @param gens 生成多项式的整数值列表（八进制表示）
 */
void ConvolutionalCode3::setGenerators(const QVector<int>& gens)
{
    if (!gens.isEmpty()) {
        m_gens = gens;
    }
}

/**
 * @brief 设置约束长度
 * @param k 约束长度（编码器移位寄存器级数+1）
 */
void ConvolutionalCode3::setConstraintLength(int k)
{
    m_constraint = qMax(2, k);
}

/**
 * @brief 设置是否启用网格终止
 * @param term true表示在编码末尾添加终止比特使编码器归零
 */
void ConvolutionalCode3::setTrellisTermination(bool term)
{
    m_terminate = term;
}

/**
 * @brief 设置Viterbi解码的回溯深度
 * @param depth 回溯深度值，影响解码延迟和性能
 */
void ConvolutionalCode3::setTracebackDepth(int depth)
{
    m_traceback = qMax(5, depth);
}

/**
 * @brief 编码输入比特序列
 *
 * 使用移位寄存器和生成多项式对输入比特进行卷积编码。
 * 每个输入比特生成n个输出比特（n为生成多项式数量）。
 *
 * @param bits 输入比特序列（0或1）
 * @return 编码后的比特序列
 */
QVector<int> ConvolutionalCode3::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> output;
    if (bits.isEmpty() || m_gens.isEmpty()) {
        emit encodeCompleted(0, 0);
        return output;
    }

    int nGenerators = m_gens.size();
    int regBits = m_constraint - 1;

    /* 移位寄存器初始化为零 */
    QVector<int> shiftReg(regBits, 0);

    /* 对每个输入比特进行编码 */
    for (int i = 0; i < bits.size(); ++i) {
        /* 右移寄存器，新比特进入最高位 */
        for (int j = regBits - 1; j > 0; --j) {
            shiftReg[j] = shiftReg[j - 1];
        }
        shiftReg[0] = bits[i];

        /* 对每个生成多项式计算输出 */
        for (int g = 0; g < nGenerators; ++g) {
            int outBit = 0;
            int gen = m_gens[g];
            /* 最高位对应当前输入 */
            outBit ^= (bits[i] & 1);
            /* 其余位对应移位寄存器 */
            for (int j = 0; j < regBits; ++j) {
                if (gen & (1 << (regBits - j))) {
                    outBit ^= shiftReg[j];
                }
            }
            output.append(outBit & 1);
        }
    }

    /* 网格终止：追加零比特使移位寄存器归零 */
    if (m_terminate) {
        for (int t = 0; t < regBits; ++t) {
            /* 移入零 */
            for (int j = regBits - 1; j > 0; --j) {
                shiftReg[j] = shiftReg[j - 1];
            }
            shiftReg[0] = 0;

            for (int g = 0; g < nGenerators; ++g) {
                int outBit = 0;
                int gen = m_gens[g];
                for (int j = 0; j < regBits; ++j) {
                    if (gen & (1 << (regBits - j))) {
                        outBit ^= shiftReg[j];
                    }
                }
                output.append(outBit & 1);
            }
        }
    }

    /* 更新统计 */
    m_stats.totalEncodes++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(bits.size(), output.size());
    return output;
}

/**
 * @brief 使用Viterbi算法解码软判决输入
 *
 * 实现硬判决Viterbi解码。对每个接收符号计算分支度量，
 * 在网格中选择幸存路径，最终回溯得到解码比特。
 *
 * @param softBits 软判决输入序列（值域0~1，>0.5视为1）
 * @return 解码后的比特序列
 */
QVector<int> ConvolutionalCode3::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> decoded;
    if (softBits.isEmpty() || m_gens.isEmpty()) {
        emit decodeCompleted(0);
        return decoded;
    }

    int nGen = m_gens.size();
    int nStates = 1 << (m_constraint - 1);
    int totalSymbols = softBits.size() / nGen;

    /* 路径度量和回溯表 */
    QVector<double> pathMetric(nStates, std::numeric_limits<double>::max());
    pathMetric[0] = 0.0;
    QVector<QVector<int>> traceback(totalSymbols, QVector<int>(nStates, 0));

    /* 逐符号处理 */
    for (int t = 0; t < totalSymbols; ++t) {
        QVector<double> newMetric(nStates, std::numeric_limits<double>::max());
        QVector<int> newTrace(nStates, 0);

        for (int state = 0; state < nStates; ++state) {
            if (pathMetric[state] >= std::numeric_limits<double>::max() / 2)
                continue;

            /* 尝试输入0和输入1 */
            for (int inBit = 0; inBit <= 1; ++inBit) {
                int prevState = (state >> 1) | (inBit << (m_constraint - 2));

                /* 计算该分支的期望输出 */
                double branchCost = 0.0;
                for (int g = 0; g < nGen; ++g) {
                    int expected = 0;
                    int reg = (state << 1) | inBit;
                    int gen = m_gens[g];
                    for (int b = 0; b < m_constraint; ++b) {
                        if (gen & (1 << b)) {
                            expected ^= ((reg >> b) & 1);
                        }
                    }
                    int idx = t * nGen + g;
                    if (idx < softBits.size()) {
                        branchCost += branchMetric(softBits[idx], expected);
                    }
                }

                double totalCost = pathMetric[prevState] + branchCost;
                if (totalCost < newMetric[state]) {
                    newMetric[state] = totalCost;
                    newTrace[state] = prevState;
                }
            }
        }
        pathMetric = newMetric;
        traceback[t] = newTrace;
    }

    /* 找到最终最小度量状态 */
    int finalState = 0;
    double minMetric = pathMetric[0];
    for (int s = 1; s < nStates; ++s) {
        if (pathMetric[s] < minMetric) {
            minMetric = pathMetric[s];
            finalState = s;
        }
    }

    /* 回溯得到解码序列 */
    int curState = finalState;
    for (int t = totalSymbols - 1; t >= 0; --t) {
        int prevState = traceback[t][curState];
        int inputBit = (curState >> (m_constraint - 2)) & 1;
        decoded.prepend(inputBit);
        curState = prevState;
    }

    /* 移除终止比特 */
    if (m_terminate && decoded.size() > (m_constraint - 1)) {
        decoded = decoded.mid(0, decoded.size() - (m_constraint - 1));
    }

    m_stats.totalDecodes++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum /
        (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(0);
    return decoded;
}

/**
 * @brief 计算分支度量（接收符号与期望值的距离）
 * @param received 接收到的软判决值
 * @param expected 期望的硬比特值（0或1）
 * @return 度量值（越小越好）
 */
int ConvolutionalCode3::branchMetric(double received, int expected) const
{
    /* 将软判决映射为硬判决，计算汉明距离 */
    int hard = (received >= 0.5) ? 1 : 0;
    return (hard == expected) ? 0 : 1;
}

/**
 * @brief 获取当前统计信息
 * @return 编解码统计结构
 */
ConvolutionalCode3::Stats ConvolutionalCode3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据
 */
void ConvolutionalCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
