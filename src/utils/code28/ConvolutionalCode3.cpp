/**
 * @file ConvolutionalCode3.cpp
 * @brief 卷积码增强实现 — 软判决Viterbi/BCJR前向后向/递归系统码
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/code28/ConvolutionalCode3.h"

#include <QElapsedTimer>

#include <cmath>
#include <limits>
#include <vector>

/** @brief 构造函数 @param parent 父对象 */
ConvolutionalCode3::ConvolutionalCode3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置生成多项式和约束长度
 *  @param polys 生成多项式列表(八进制表示)
 *  @param constraintLength 约束长度
 */
void ConvolutionalCode3::setGeneratorPolynomials(const QVector<int>& polys,
                                                  int constraintLength)
{
    m_polys = polys;
    m_constraint = qMax(2, constraintLength);
    m_numStates = 1 << (m_constraint - 1);
}

/** @brief 设置递归系统码的反馈多项式 @param feedbackPoly 反馈多项式(八进制) */
void ConvolutionalCode3::setRecursivePolynomial(int feedbackPoly)
{
    m_feedbackPoly = feedbackPoly;
    m_recursive = (feedbackPoly != 0);
}

/** @brief 设置是否进行尾部终止 @param terminate true则添加终止比特 */
void ConvolutionalCode3::setTermination(bool terminate)
{
    m_terminated = terminate;
}

/** @brief 获取约束长度 @return 约束长度 */
int ConvolutionalCode3::constraintLength() const
{
    return m_constraint;
}

/** @brief 获取码率倒数(输出比特数/输入比特数) @return 1除以码率 */
int ConvolutionalCode3::codeRate() const
{
    return m_polys.size();
}

/** @brief 安全读取softBits元素(越界返回0) @param v 向量 @param idx 索引 @return 元素值或0 */
static double softBits_safe(const QVector<double>& v, int idx)
{
    return (idx >= 0 && idx < v.size()) ? v[idx] : 0.0;
}

/** @brief 编码输入比特序列 @param bits 输入比特 @return 编码输出比特 */
QVector<int> ConvolutionalCode3::encode(const QVector<int>& bits) const
{
    QElapsedTimer timer;
    timer.start();

    QVector<int> encoded;
    if (m_polys.isEmpty()) return encoded;

    int state = 0;
    int nPolys = m_polys.size();

    /* 编码输入数据 */
    for (int bit : bits) {
        int b = bit & 1;
        int reg = (state >> 1) | (b << (m_constraint - 1));

        /* 如果是递归码，先计算反馈 */
        if (m_recursive && m_feedbackPoly != 0) {
            int fb = reg & m_feedbackPoly;
            int fbBit = 0;
            while (fb) { fbBit ^= (fb & 1); fb >>= 1; }
            reg = (state >> 1) | (fbBit << (m_constraint - 1));
        }

        for (int poly : m_polys) {
            int val = reg & poly;
            int outBit = 0;
            while (val) { outBit ^= (val & 1); val >>= 1; }
            encoded.append(outBit);
        }
        state = (reg >> 1) & (m_numStates - 1);
    }

    /* 尾部终止: 将移位寄存器清零 */
    if (m_terminated) {
        for (int i = 0; i < m_constraint - 1; ++i) {
            int reg = state >> 1;
            if (m_recursive && m_feedbackPoly != 0) {
                int fb = (reg | (0 << (m_constraint - 1))) & m_feedbackPoly;
                int fbBit = 0;
                while (fb) { fbBit ^= (fb & 1); fb >>= 1; }
                reg = (state >> 1) | (fbBit << (m_constraint - 1));
            }
            for (int poly : m_polys) {
                int val = reg & poly;
                int outBit = 0;
                while (val) { outBit ^= (val & 1); val >>= 1; }
                encoded.append(outBit);
            }
            state = (reg >> 1) & (m_numStates - 1);
        }
    }

    m_stats.totalEncodes++;
    m_stats.totalBitsProcessed += bits.size();
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalEncodes + m_stats.totalDecodes);
    return encoded;
}

/** @brief 软判决Viterbi解码 @param softBits 软比特(LLR或浮点值) @return 解码比特 */
QVector<int> ConvolutionalCode3::decodeViterbi(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    int nPolys = m_polys.size();
    int nSteps = softBits.size() / nPolys;
    QVector<int> decoded;

    if (nSteps == 0 || m_polys.isEmpty()) return decoded;

    /* 初始化路径度量 */
    double negInf = -std::numeric_limits<double>::infinity();
    std::vector<double> prevMetric(m_numStates, negInf);
    std::vector<double> currMetric(m_numStates, negInf);
    prevMetric[0] = 0.0;

    /* 幸存路径回溯表 */
    std::vector<std::vector<int>> path(nSteps,
        std::vector<int>(m_numStates, 0));

    for (int t = 0; t < nSteps; ++t) {
        std::fill(currMetric.begin(), currMetric.end(), negInf);

        for (int s = 0; s < m_numStates; ++s) {
            if (prevMetric[s] == negInf) continue;

            for (int input = 0; input <= 1; ++input) {
                int reg = (s >> 1) | (input << (m_constraint - 1));
                int nextState = (s >> 1) & (m_numStates - 1);

                /* 计算分支度量 */
                double bm = 0.0;
                for (int p = 0; p < nPolys; ++p) {
                    int val = reg & m_polys[p];
                    int outBit = 0;
                    while (val) { outBit ^= (val & 1); val >>= 1; }
                    double received = softBits[t * nPolys + p];
                    bm += (outBit == 0) ? received : -received;
                }

                double newMetric = prevMetric[s] + bm;
                if (newMetric > currMetric[nextState]) {
                    currMetric[nextState] = newMetric;
                    path[t][nextState] = s;
                }
            }
        }
        prevMetric.swap(currMetric);
    }

    /* 找最终状态(如果终止则从状态0回溯) */
    int finalState = 0;
    double bestMetric = negInf;
    if (!m_terminated) {
        for (int s = 0; s < m_numStates; ++s) {
            if (prevMetric[s] > bestMetric) {
                bestMetric = prevMetric[s];
                finalState = s;
            }
        }
    }

    /* 回溯 */
    decoded.fill(0, nSteps);
    int state = finalState;
    for (int t = nSteps - 1; t >= 0; --t) {
        int prevState = path[t][state];
        decoded[t] = (state != prevState) ?
            ((prevState << 1) & (m_numStates - 1)) != state ?
            (state & (1 << (m_constraint - 2))) ? 1 : 0 :
            ((prevState >> 1) == state) ? 0 : 1 : 0;
        state = prevState;
    }

    /* 去除终止比特 */
    int infoLen = nSteps;
    if (m_terminated) {
        infoLen = qMax(0, nSteps - (m_constraint - 1));
    }

    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += nSteps;
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalEncodes + m_stats.totalDecodes);
    emit decodeComplete(infoLen, 0.0);
    return decoded.mid(0, infoLen);
}

/** @brief BCJR前向后向算法解码 @param llr 对数似然比序列 @return 后验LLR */
QVector<double> ConvolutionalCode3::decodeBCJR(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    int nPolys = m_polys.size();
    int nSteps = llr.size() / nPolys;
    QVector<double> appLlr;

    if (nSteps == 0 || m_polys.isEmpty()) return appLlr;

    double negInf = -1e9;

    /* 前向度量 alpha[t][state] */
    std::vector<std::vector<double>> alpha(nSteps + 1,
        std::vector<double>(m_numStates, negInf));
    alpha[0][0] = 0.0;

    /* 前向递推 */
    for (int t = 0; t < nSteps; ++t) {
        for (int s = 0; s < m_numStates; ++s) {
            if (alpha[t][s] < negInf + 1e6) continue;
            for (int input = 0; input <= 1; ++input) {
                int reg = (s >> 1) | (input << (m_constraint - 1));
                int ns = (s >> 1) & (m_numStates - 1);
                double gm = 0.0;
                for (int p = 0; p < nPolys; ++p) {
                    int val = reg & m_polys[p];
                    int outBit = 0;
                    while (val) { outBit ^= (val & 1); val >>= 1; }
                    int idx = t * nPolys + p;
                    double rx = (idx < llr.size()) ? llr[idx] : 0.0;
                    gm += (outBit == 0) ? rx : -rx;
                }
                double newVal = alpha[t][s] + gm;
                if (newVal > alpha[t + 1][ns]) {
                    alpha[t + 1][ns] = newVal;
                }
            }
        }
    }

    /* 后向度量 beta[t][state] */
    std::vector<std::vector<double>> beta(nSteps + 1,
        std::vector<double>(m_numStates, negInf));
    if (m_terminated) {
        beta[nSteps][0] = 0.0;
    } else {
        for (int s = 0; s < m_numStates; ++s) {
            beta[nSteps][s] = 0.0;
        }
    }

    /* 后向递推 */
    for (int t = nSteps - 1; t >= 0; --t) {
        for (int s = 0; s < m_numStates; ++s) {
            for (int input = 0; input <= 1; ++input) {
                int reg = (s >> 1) | (input << (m_constraint - 1));
                int ns = (s >> 1) & (m_numStates - 1);
                double gm = 0.0;
                for (int p = 0; p < nPolys; ++p) {
                    int val = reg & m_polys[p];
                    int outBit = 0;
                    while (val) { outBit ^= (val & 1); val >>= 1; }
                    int idx = t * nPolys + p;
                    double rx = (idx < llr.size()) ? llr[idx] : 0.0;
                    gm += (outBit == 0) ? rx : -rx;
                }
                double newVal = beta[t + 1][ns] + gm;
                if (newVal > beta[t][s]) {
                    beta[t][s] = newVal;
                }
            }
        }
    }

    /* 计算后验LLR */
    appLlr.fill(0.0, nSteps);
    for (int t = 0; t < nSteps; ++t) {
        double maxZero = negInf;
        double maxOne = negInf;
        for (int s = 0; s < m_numStates; ++s) {
            if (alpha[t][s] < negInf + 1e6) continue;
            for (int input = 0; input <= 1; ++input) {
                int reg = (s >> 1) | (input << (m_constraint - 1));
                int ns = (s >> 1) & (m_numStates - 1);
                double gm = 0.0;
                for (int p = 0; p < nPolys; ++p) {
                    int val = reg & m_polys[p];
                    int outBit = 0;
                    while (val) { outBit ^= (val & 1); val >>= 1; }
                    int idx = t * nPolys + p;
                    double rx = (idx < llr.size()) ? llr[idx] : 0.0;
                    gm += (outBit == 0) ? rx : -rx;
                }
                double metric = alpha[t][s] + gm + beta[t + 1][ns];
                if (input == 0) {
                    maxZero = qMax(maxZero, metric);
                } else {
                    maxOne = qMax(maxOne, metric);
                }
            }
        }
        appLlr[t] = maxZero - maxOne;
    }

    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += nSteps;
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = m_timeSum /
        qMax(1, m_stats.totalEncodes + m_stats.totalDecodes);
    emit decodeComplete(nSteps, 0.0);
    return appLlr;
}

/** @brief 重置所有统计计数器 */
void ConvolutionalCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
