/**
 * @file TurboCode4.cpp
 * @brief Turbo码4实现 — 双二进制+自交织
 *
 * Turbo码编解码器实现，支持双二进制分量码和自交织器。
 * 解码采用LOG-MAP算法迭代软判决，适用于高可靠性通信场景。
 */

#include "utils/code45/TurboCode4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
TurboCode4::TurboCode4(QObject* parent)
    : QObject(parent)
{
    /* 默认生成多项式: [1 1 1] / [1 0 1] (八进制7/5) */
    m_generator = {1, 1, 1};
}

/**
 * @brief 设置约束长度
 * @param K 约束长度，决定卷积编码器记忆深度
 */
void TurboCode4::setConstraintLength(int K)
{
    m_K = qMax(2, K);
    /* 更新默认生成多项式长度 */
    m_generator.resize(m_K, 1);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter Turbo解码最大迭代次数
 */
void TurboCode4::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(1, maxIter);
}

/**
 * @brief 设置交织器
 * @param interleaver 交织图案，定义比特重排顺序
 */
void TurboCode4::setInterleaver(const QVector<int>& interleaver)
{
    m_interleaver = interleaver;
}

/**
 * @brief Turbo编码
 * @param bits 输入信息比特序列
 * @return 编码后的比特序列（系统位+校验位）
 *
 * 双二进制编码: 两个分量编码器并行编码，
 * 第二个编码器对交织后的序列编码。
 */
QVector<int> TurboCode4::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    const int n = bits.size();
    if (n == 0) return QVector<int>();

    /* 生成默认交织器 */
    if (m_interleaver.size() != n) {
        m_interleaver.resize(n);
        for (int i = 0; i < n; ++i) {
            m_interleaver[i] = i;
        }
        /* 简单交织: 奇偶分离 */
        int idx = 0;
        for (int i = 0; i < n; i += 2) {
            m_interleaver[idx++] = i;
        }
        for (int i = 1; i < n; i += 2) {
            m_interleaver[idx++] = i;
        }
    }

    /* 分量编码器1: 直接编码 */
    QVector<int> parity1(n, 0);
    QVector<int> state(m_K - 1, 0);

    for (int i = 0; i < n; ++i) {
        /* 计算校验输出 */
        int feedback = bits[i] ^ state[0];
        int parity = feedback;
        for (int j = 1; j < m_K - 1; ++j) {
            if (j < m_generator.size()) {
                parity ^= (state[j] & m_generator[j]);
            }
        }
        parity1[i] = parity;

        /* 状态转移 */
        for (int j = m_K - 2; j > 0; --j) {
            state[j] = state[j - 1];
        }
        state[0] = feedback;
    }

    /* 交织后编码器2 */
    QVector<int> interleaved(n);
    for (int i = 0; i < n; ++i) {
        int idx = m_interleaver[i];
        interleaved[i] = (idx >= 0 && idx < n) ? bits[idx] : 0;
    }

    QVector<int> parity2(n, 0);
    state.fill(0);

    for (int i = 0; i < n; ++i) {
        int feedback = interleaved[i] ^ state[0];
        int parity = feedback;
        for (int j = 1; j < m_K - 1; ++j) {
            if (j < m_generator.size()) {
                parity ^= (state[j] & m_generator[j]);
            }
        }
        parity2[i] = parity;

        for (int j = m_K - 2; j > 0; --j) {
            state[j] = state[j - 1];
        }
        state[0] = feedback;
    }

    /* 复用输出: 系统位 + 校验1 + 校验2 */
    QVector<int> encoded;
    encoded.reserve(n * 3);
    for (int i = 0; i < n; ++i) {
        encoded.append(bits[i]);
        encoded.append(parity1[i]);
        encoded.append(parity2[i]);
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalEncodes++;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return encoded;
}

/**
 * @brief Turbo解码
 * @param softBits 软比特输入（LLR值）
 * @return 硬判决后的信息比特
 *
 * 迭代LOG-MAP解码:
 * 1. 分量解码器1处理原始顺序
 * 2. 交织外信息传递给解码器2
 * 3. 解码器2处理交织顺序
 * 4. 解交织外信息反馈给解码器1
 * 5. 重复直到收敛或达到最大迭代次数
 */
QVector<int> TurboCode4::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    const int totalLen = softBits.size();
    const int n = totalLen / 3; /* 每个信息位对应3个编码位 */

    if (n == 0 || m_interleaver.size() != n) {
        /* 自动生成交织器 */
        m_interleaver.resize(n);
        int idx = 0;
        for (int i = 0; i < n; i += 2) m_interleaver[idx++] = i;
        for (int i = 1; i < n; i += 2) m_interleaver[idx++] = i;
    }

    /* 分解输入为系统位、校验1、校验2 */
    QVector<double> systematic(n, 0.0);
    QVector<double> parity1(n, 0.0);
    QVector<double> parity2(n, 0.0);

    for (int i = 0; i < n && i * 3 + 2 < totalLen; ++i) {
        systematic[i] = softBits[i * 3];
        parity1[i] = softBits[i * 3 + 1];
        parity2[i] = softBits[i * 3 + 2];
    }

    /* 先验信息初始化为零 */
    QVector<double> prior1(n, 0.0);
    QVector<double> prior2(n, 0.0);

    bool converged = false;
    int iter = 0;

    for (iter = 0; iter < m_maxIterations; ++iter) {
        /* ---- 分量解码器1 ---- */
        QVector<double> extrinsic1 = constituentDecode(
            systematic, parity1, prior1, true);

        /* 交织外信息 */
        QVector<double> interleavedExt(n, 0.0);
        for (int i = 0; i < n; ++i) {
            int idx = m_interleaver[i];
            if (idx >= 0 && idx < n) {
                interleavedExt[i] = extrinsic1[idx];
            }
        }

        /* ---- 分量解码器2 ---- */
        QVector<double> interleavedSys(n, 0.0);
        for (int i = 0; i < n; ++i) {
            int idx = m_interleaver[i];
            if (idx >= 0 && idx < n) {
                interleavedSys[i] = systematic[idx];
            }
        }

        QVector<double> extrinsic2 = constituentDecode(
            interleavedSys, parity2, interleavedExt, true);

        /* 解交织外信息 */
        for (int i = 0; i < n; ++i) {
            int idx = m_interleaver[i];
            if (idx >= 0 && idx < n) {
                prior1[idx] = extrinsic2[i];
            }
        }

        /* 收敛检查: 比较两次迭代结果 */
        if (iter > 0) {
            bool same = true;
            for (int i = 0; i < n && same; ++i) {
                double llr = systematic[i] + prior1[i];
                int prevBit = (prior2[i] > 0) ? 0 : 1;
                int curBit = (llr > 0) ? 0 : 1;
                if (prevBit != curBit) same = false;
            }
            if (same) { converged = true; break; }
        }

        prior2 = prior1;
    }

    /* 硬判决 */
    QVector<int> decoded(n, 0);
    for (int i = 0; i < n; ++i) {
        double llr = systematic[i] + prior1[i];
        decoded[i] = (llr > 0) ? 0 : 1;
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodes++;
    m_stats.totalIterations += iter;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(iter, converged);
    return decoded;
}

/**
 * @brief 分量卷积码解码器(LOG-MAP近似)
 * @param systematic 系统位LLR
 * @param parity 校验位LLR
 * @param prior 先验信息LLR
 * @param terminated 是否尾部归零
 * @return 外信息LLR
 *
 * 简化的LOG-MAP解码，使用Max-Log-MAP近似。
 */
QVector<double> TurboCode4::constituentDecode(
    const QVector<double>& systematic,
    const QVector<double>& parity,
    const QVector<double>& prior,
    bool terminated) const
{
    const int n = systematic.size();
    const int numStates = 1 << (m_K - 1);

    /* Alpha前向度量 */
    QVector<QVector<double>> alpha(n + 1, QVector<double>(numStates, -1e10));
    alpha[0][0] = 0.0;

    /* Beta后向度量 */
    QVector<QVector<double>> beta(n + 1, QVector<double>(numStates, -1e10));
    beta[n][0] = 0.0;

    /* Gamma分支度量 */
    auto gamma = [&](int state, int input, int t) -> double {
        int nextState = ((state << 1) | input) & (numStates - 1);
        /* 计算期望校验输出 */
        int fb = input ^ (state & 1);
        int expectedParity = fb;
        for (int j = 1; j < m_K - 1; ++j) {
            if (j < m_generator.size()) {
                expectedParity ^= ((state >> j) & 1) & m_generator[j];
            }
        }
        /* 分支度量 = 系统位 + 校验位 + 先验 */
        double llr = (input == 0 ? 1.0 : -1.0) * systematic[t]
                     + (expectedParity == 0 ? 1.0 : -1.0) * parity[t];
        if (t < prior.size()) {
            llr += prior[t];
        }
        return llr;
    };

    /* 前向递推 */
    for (int t = 0; t < n; ++t) {
        for (int s = 0; s < numStates; ++s) {
            for (int in = 0; in <= 1; ++in) {
                int ns = ((s << 1) | in) & (numStates - 1);
                double g = gamma(s, in, t);
                double val = alpha[t][s] + g;
                if (val > alpha[t + 1][ns]) {
                    alpha[t + 1][ns] = val;
                }
            }
        }
    }

    /* 后向递推 */
    for (int t = n - 1; t >= 0; --t) {
        for (int s = 0; s < numStates; ++s) {
            for (int in = 0; in <= 1; ++in) {
                int ps = ((s >> 1) | (in << (m_K - 2))) & (numStates - 1);
                double g = gamma(ps, s & 1, t);
                double val = beta[t + 1][s] + g;
                if (val > beta[t][ps]) {
                    beta[t][ps] = val;
                }
            }
        }
    }

    /* 计算外信息 */
    QVector<double> extrinsic(n, 0.0);
    for (int t = 0; t < n; ++t) {
        double max0 = -1e10, max1 = -1e10;
        for (int s = 0; s < numStates; ++s) {
            for (int in = 0; in <= 1; ++in) {
                int ns = ((s << 1) | in) & (numStates - 1);
                double g = gamma(s, in, t);
                double val = alpha[t][s] + g + beta[t + 1][ns];
                if (in == 0) max0 = qMax(max0, val);
                else max1 = qMax(max1, val);
            }
        }
        /* 外信息 = LLR - 系统位 - 先验 */
        extrinsic[t] = max0 - max1;
        if (t < systematic.size()) extrinsic[t] -= systematic[t];
        if (t < prior.size()) extrinsic[t] -= prior[t];
    }

    return extrinsic;
}

/**
 * @brief 计算外信息与先验的sigma值
 * @param extrinsic 外信息LLR
 * @param prior 先验LLR
 * @return 组合可靠性度量
 */
double TurboCode4::sigmaCompute(double extrinsic, double prior) const
{
    double combined = extrinsic + prior;
    return 1.0 / (1.0 + qExp(-qFabs(combined)));
}

/**
 * @brief 重置所有统计信息
 */
void TurboCode4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
