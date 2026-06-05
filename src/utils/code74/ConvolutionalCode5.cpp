/**
 * @file ConvolutionalCode5.cpp
 * @brief 卷积码编解码器实现
 *
 * 实现卷积编码和Viterbi解码算法，支持自定义生成多项式
 * 和约束长度。适用于通信系统的前向纠错编码。
 */

#include "utils/code74/ConvolutionalCode5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
ConvolutionalCode5::ConvolutionalCode5(QObject* parent)
    : QObject(parent)
{
    // 默认生成多项式（常见(171,133)八进制）
    m_gens = {0171, 0133};
}

/**
 * @brief 设置生成多项式
 * @param gens 生成多项式向量（八进制表示）
 */
void ConvolutionalCode5::setGenerators(const QVector<int>& gens)
{
    if (!gens.isEmpty()) {
        m_gens = gens;
    }
}

/**
 * @brief 设置约束长度
 * @param k 约束长度（编码器记忆深度+1）
 */
void ConvolutionalCode5::setConstraintLength(int k)
{
    m_constraint = qBound(3, k, 15);
}

/**
 * @brief 编码信息比特
 * @param bits 输入信息比特
 * @return 编码后的比特序列
 */
QVector<int> ConvolutionalCode5::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    if (bits.isEmpty() || m_gens.isEmpty()) return QVector<int>();

    int n = bits.size();
    int numOut = m_gens.size();
    int tailBits = m_constraint - 1;

    // 移位寄存器
    int reg = 0;
    QVector<int> encoded;
    encoded.reserve((n + tailBits) * numOut);

    for (int i = 0; i < n + tailBits; ++i) {
        int bit = (i < n) ? (bits[i] & 1) : 0; // 尾部添加0
        reg = ((reg << 1) | bit) & ((1 << m_constraint) - 1);

        for (int g = 0; g < numOut; ++g) {
            // 计算生成多项式的输出
            int out = 0;
            int poly = m_gens[g];
            int temp = reg & poly;
            // 计算奇偶校验
            while (temp) {
                out ^= (temp & 1);
                temp >>= 1;
            }
            encoded.append(out);
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit encodeCompleted(n, encoded.size());
    return encoded;
}

/**
 * @brief Viterbi解码软判决比特
 * @param softBits 输入软判决值
 * @return 解码后的硬判决比特
 */
QVector<int> ConvolutionalCode5::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    if (softBits.isEmpty()) return QVector<int>();

    int numStates = 1 << (m_constraint - 1);
    int numOut = m_gens.size();
    int numSteps = softBits.size() / numOut;

    // 路径度量和回溯
    QVector<double> metrics(numStates, 1e18);
    QVector<QVector<int>> trellis(numSteps, QVector<int>(numStates, 0));
    metrics[0] = 0.0; // 初始状态为0

    // Viterbi逐阶段处理
    for (int step = 0; step < numSteps; ++step) {
        QVector<double> newMetrics(numStates, 1e18);

        for (int state = 0; state < numStates; ++state) {
            if (metrics[state] >= 1e17) continue;

            for (int input = 0; input <= 1; ++input) {
                // 计算下一状态
                int nextState = ((state << 1) | input) & (numStates - 1);
                // 在step > state的对应时刻才可能（前向约束）

                // 计算编码器输出
                int reg = ((state << 1) | input) & ((1 << m_constraint) - 1);
                double branchMetric = 0.0;
                for (int g = 0; g < numOut; ++g) {
                    int out = 0;
                    int poly = m_gens[g];
                    int temp = reg & poly;
                    while (temp) { out ^= (temp & 1); temp >>= 1; }

                    // 软判决距离：期望值与接收值的差
                    int idx = step * numOut + g;
                    double expected = out ? 1.0 : -1.0;
                    double received = (idx < softBits.size()) ? softBits[idx] : 0.0;
                    double diff = expected - received;
                    branchMetric += diff * diff;
                }

                double totalMetric = metrics[state] + branchMetric;
                if (totalMetric < newMetrics[nextState]) {
                    newMetrics[nextState] = totalMetric;
                    trellis[step][nextState] = state;
                }
            }
        }
        metrics = newMetrics;
    }

    // 回溯
    int bestState = 0;
    double bestMetric = metrics[0];
    for (int s = 1; s < numStates; ++s) {
        if (metrics[s] < bestMetric) {
            bestMetric = metrics[s];
            bestState = s;
        }
    }

    QVector<int> decoded;
    decoded.reserve(numSteps);
    int state = bestState;
    for (int step = numSteps - 1; step >= 0; --step) {
        int prevState = trellis[step][state];
        int inputBit = state & 1;
        decoded.prepend(inputBit);
        state = prevState;
    }

    // 移除尾部比特
    int tailBits = m_constraint - 1;
    if (decoded.size() > tailBits) {
        decoded.resize(decoded.size() - tailBits);
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return decoded;
}

/**
 * @brief 计算自由距离
 * @return 卷积码的自由距离（简化估计）
 */
int ConvolutionalCode5::freeDistance() const
{
    // 简化：通过遍历短序列估计自由距离
    int numStates = 1 << (m_constraint - 1);
    int minDist = 100;

    for (int len = 1; len <= 2 * m_constraint; ++len) {
        // 枚举所有len位序列（以非零开始）
        for (int seq = 1; seq < (1 << len); ++seq) {
            int reg = 0;
            int weight = 0;
            for (int i = 0; i < len + m_constraint - 1; ++i) {
                int bit = (i < len) ? ((seq >> i) & 1) : 0;
                reg = ((reg << 1) | bit) & ((1 << m_constraint) - 1);
                for (int g = 0; g < m_gens.size(); ++g) {
                    int out = 0;
                    int temp = reg & m_gens[g];
                    while (temp) { out ^= (temp & 1); temp >>= 1; }
                    weight += out;
                }
            }
            if (weight > 0 && weight < minDist) {
                minDist = weight;
            }
        }
    }
    return minDist;
}

/**
 * @brief 重置统计信息
 */
void ConvolutionalCode5::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
