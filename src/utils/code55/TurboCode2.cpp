/**
 * @file TurboCode2.cpp
 * @brief Turbo码编解码实现 — 并行级联卷积码 + BCJR迭代译码
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现 Turbo 码的编码和迭代译码。
 * 编码器由两个并行级联的递归系统卷积码（RSC）编码器组成，
 * 中间通过交织器连接。
 * 译码器使用 BCJR（前向-后向）算法进行迭代软输入软输出译码。
 */

#include "utils/code55/TurboCode2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 Turbo 码参数
 * @param parent 父QObject对象
 */
TurboCode2::TurboCode2(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("TurboCode2"));
    generateInterleaver(m_blockSize);
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置信息块大小
 *
 * 改变块大小会自动重新生成交织器。
 *
 * @param n 信息比特数，必须 >= 1
 */
void TurboCode2::setBlockSize(int n)
{
    m_blockSize = qMax(1, n);
    generateInterleaver(m_blockSize);
}

/**
 * @brief 设置迭代译码次数
 *
 * 迭代次数越多，译码性能越好，但延迟越高。
 * 通常 5~8 次迭代即可获得良好性能。
 *
 * @param iter 迭代次数
 */
void TurboCode2::setNumIterations(int iter)
{
    m_numIter = qMax(1, iter);
}

// ──────────────────────────────────────────────
// 编码
// ──────────────────────────────────────────────

/**
 * @brief Turbo 码编码
 *
 * 编码过程：
 * 1. 第一个 RSC 编码器直接对输入比特编码
 * 2. 交织器对输入比特重排
 * 3. 第二个 RSC 编码器对交织后的比特编码
 * 4. 输出 = 系统比特 + 校验1 + 校验2
 *
 * @param bits 输入信息比特
 * @return 编码输出比特（rate 1/3）
 */
QVector<int> TurboCode2::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    const int n = bits.size();
    if (n == 0) return {};

    // RSC 编码器（约束长度3，生成多项式 [1, 5/7]）
    // 反馈多项式 = 7（八进制），前馈多项式 = 5（八进制）
    auto rscEncode = [](const QVector<int>& input, bool terminated) -> QVector<int> {
        QVector<int> parity;
        int state = 0;
        const int regLen = 2;

        for (int i = 0; i < input.size(); ++i) {
            int bit = input[i] & 1;
            int feedback = bit ^ ((state >> 1) & 1) ^ (state & 1);
            int newState = ((state << 1) | feedback) & ((1 << regLen) - 1);
            int parityBit = feedback ^ ((newState >> 1) & 1);
            parity.append(parityBit & 1);
            state = newState;
        }

        // 终止处理
        if (terminated) {
            for (int t = 0; t < regLen; ++t) {
                int feedback = 0 ^ ((state >> 1) & 1) ^ (state & 1);
                int newState = ((state << 1) | feedback) & ((1 << regLen) - 1);
                int parityBit = feedback ^ ((newState >> 1) & 1);
                parity.append(parityBit & 1);
                state = newState;
            }
        }

        return parity;
    };

    // 第一路 RSC 校验
    QVector<int> parity1 = rscEncode(bits, true);

    // 交织
    QVector<int> interleaved(n);
    for (int i = 0; i < n; ++i) {
        interleaved[i] = bits[m_interleaver[i % m_interleaver.size()]];
    }

    // 第二路 RSC 校验
    QVector<int> parity2 = rscEncode(interleaved, true);

    // 复用输出：[sys, par1, par2, sys, par1, par2, ...]
    QVector<int> output;
    output.reserve(n * 3);
    for (int i = 0; i < n; ++i) {
        output.append(bits[i]);
        output.append((i < parity1.size()) ? parity1[i] : 0);
        output.append((i < parity2.size()) ? parity2[i] : 0);
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return output;
}

// ──────────────────────────────────────────────
// BCJR 迭代译码
// ──────────────────────────────────────────────

/**
 * @brief Turbo 码迭代译码
 *
 * 使用两个 BCJR 译码器交替工作：
 * 1. DEC1 处理系统比特 + 校验1 + 先验信息
 * 2. DEC2 处理交织后的系统比特 + 校验2 + DEC1的外信息
 * 3. 重复 m_numIter 次迭代
 * 4. 最终对 DEC2 输出做硬判决
 *
 * @param softBits 软判决输入 LLR
 * @return 译码后的硬比特
 */
QVector<int> TurboCode2::decode(const QVector<double>& softBits)
{
    QElapsedTimer timer;
    timer.start();

    const int n = softBits.size() / 3;
    if (n <= 0) return {};

    // 提取三路信息
    QVector<double> sys(n), par1(n), par2(n);
    for (int i = 0; i < n; ++i) {
        sys[i]  = (i * 3 < softBits.size())     ? softBits[i * 3]     : 0.0;
        par1[i] = (i * 3 + 1 < softBits.size()) ? softBits[i * 3 + 1] : 0.0;
        par2[i] = (i * 3 + 2 < softBits.size()) ? softBits[i * 3 + 2] : 0.0;
    }

    // 交织后的系统比特
    QVector<double> intSys(n);
    for (int i = 0; i < n; ++i) {
        intSys[i] = sys[m_interleaver[i % m_interleaver.size()]];
    }

    // 先验信息初始化为零
    QVector<double> prior1(n, 0.0);
    QVector<double> prior2(n, 0.0);

    QVector<double> extrinsic1, extrinsic2;
    double finalLlr = 0.0;

    for (int iter = 0; iter < m_numIter; ++iter) {
        // DEC1：处理 sys + par1 + prior1
        extrinsic1 = bcjrDecode(sys, par1, prior1, true);

        // 交织外信息作为 DEC2 的先验
        for (int i = 0; i < n; ++i) {
            prior2[i] = extrinsic1[m_interleaver[i % m_interleaver.size()]];
        }

        // DEC2：处理 intSys + par2 + prior2
        extrinsic2 = bcjrDecode(intSys, par2, prior2, true);

        // 解交织 DEC2 外信息作为下一轮 DEC1 的先验
        for (int i = 0; i < n; ++i) {
            int deintIdx = m_interleaver[i % m_interleaver.size()];
            prior1[i] = (deintIdx < extrinsic2.size()) ? extrinsic2[deintIdx] : 0.0;
        }
    }

    // 最终硬判决
    QVector<int> decoded(n, 0);
    finalLlr = 0.0;
    for (int i = 0; i < n; ++i) {
        double llr = sys[i] + prior1[i];
        decoded[i] = (llr > 0.0) ? 1 : 0;
        finalLlr += qAbs(llr);
    }
    finalLlr /= n;

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    emit decodeCompleted(m_numIter, finalLlr);
    return decoded;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含编码/译码次数和平均耗时的Stats结构
 */
TurboCode2::Stats TurboCode2::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void TurboCode2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 交织器生成
// ──────────────────────────────────────────────

/**
 * @brief 生成交织器排列
 *
 * 使用基于素数的伪随机交织器。
 * 对于长度为 n 的交织器，生成一个 0~n-1 的排列。
 *
 * @param n 交织器长度
 */
void TurboCode2::generateInterleaver(int n)
{
    m_interleaver.resize(n);
    for (int i = 0; i < n; ++i) {
        m_interleaver[i] = i;
    }

    // 基于 S-random 准则的交织器
    // 使用确定性种子
    unsigned int seed = 42;
    for (int i = n - 1; i > 0; --i) {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        int j = seed % (i + 1);
        std::swap(m_interleaver[i], m_interleaver[j]);
    }
}

// ──────────────────────────────────────────────
// 私有方法 — BCJR 译码
// ──────────────────────────────────────────────

/**
 * @brief BCJR（前向-后向）算法实现
 *
 * Log-MAP 简化版本，使用 max* 运算符。
 * 输出外信息（extrinsic）用于迭代译码。
 *
 * @param sys 系统比特 LLR
 * @param par 校验比特 LLR
 * @param prior 先验信息 LLR
 * @param terminated 编码器是否终止
 * @return 外信息 LLR
 */
QVector<double> TurboCode2::bcjrDecode(const QVector<double>& sys,
                                         const QVector<double>& par,
                                         const QVector<double>& prior,
                                         bool terminated)
{
    const int n = sys.size();
    if (n == 0) return {};

    const int numStates = 4; // 2^(constraint-1) = 4 for constraint=3
    const double INF = 30.0; // 对数域无穷大

    // 初始化
    QVector<QVector<double>> alpha(n + 1, QVector<double>(numStates, -INF));
    QVector<QVector<double>> beta(n + 1, QVector<double>(numStates, -INF));

    alpha[0][0] = 0.0;
    if (terminated) {
        beta[n][0] = 0.0;
    } else {
        for (int s = 0; s < numStates; ++s) {
            beta[n][s] = 0.0;
        }
    }

    // 分支度量计算和前向递推
    auto branchMetric = [&](int state, int input, double sysVal, double parVal, double priorVal) -> double {
        Q_UNUSED(state);
        return (input == 1 ? sysVal : -sysVal) * 0.5 +
               priorVal * 0.5;
    };

    // 状态转移表（简化 RSC）
    // nextState[state][input], outputParity[state][input]
    const int nextState[4][2] = {{0, 2}, {2, 0}, {1, 3}, {3, 1}};
    const int outputParity[4][2] = {{0, 1}, {1, 0}, {1, 0}, {0, 1}};

    // 前向递推
    for (int t = 0; t < n; ++t) {
        for (int s = 0; s < numStates; ++s) {
            for (int in = 0; in < 2; ++in) {
                int prevS = s; // 简化：需要反向查找
                // 正确做法：遍历所有前一状态
            }
        }

        // 简化实现：遍历所有 (prevState, input) 对
        for (int ps = 0; ps < numStates; ++ps) {
            for (int in = 0; in < 2; ++in) {
                int ns = nextState[ps][in];
                double bm = branchMetric(ps, in, sys[t], par[t], prior[t]);
                // 加上校验匹配度
                double parMatch = (outputParity[ps][in] == 1) ? par[t] * 0.5 : -par[t] * 0.5;
                bm += parMatch;

                double val = alpha[t][ps] + bm;
                if (val > alpha[t + 1][ns]) {
                    alpha[t + 1][ns] = val;
                }
            }
        }
    }

    // 后向递推
    for (int t = n - 1; t >= 0; --t) {
        for (int ps = 0; ps < numStates; ++ps) {
            for (int in = 0; in < 2; ++in) {
                int ns = nextState[ps][in];
                double bm = branchMetric(ps, in, sys[t], par[t], prior[t]);
                double parMatch = (outputParity[ps][in] == 1) ? par[t] * 0.5 : -par[t] * 0.5;
                bm += parMatch;

                double val = beta[t + 1][ns] + bm;
                if (val > beta[t][ps]) {
                    beta[t][ps] = val;
                }
            }
        }
    }

    // 计算外信息
    QVector<double> extrinsic(n, 0.0);
    for (int t = 0; t < n; ++t) {
        double max1 = -INF;
        double max0 = -INF;

        for (int ps = 0; ps < numStates; ++ps) {
            int ns1 = nextState[ps][1];
            double bm1 = (outputParity[ps][1] == 1 ? par[t] * 0.5 : -par[t] * 0.5);
            double val1 = alpha[t][ps] + bm1 + beta[t + 1][ns1];
            if (val1 > max1) max1 = val1;

            int ns0 = nextState[ps][0];
            double bm0 = (outputParity[ps][0] == 1 ? par[t] * 0.5 : -par[t] * 0.5);
            double val0 = alpha[t][ps] + bm0 + beta[t + 1][ns0];
            if (val0 > max0) max0 = val0;
        }

        extrinsic[t] = max1 - max0 - sys[t] - prior[t];
    }

    return extrinsic;
}
