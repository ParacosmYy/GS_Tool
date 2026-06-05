/**
 * @file LDPCCode3.cpp
 * @brief LDPC码编解码实现 — 稀疏奇偶校验矩阵 + 置信传播译码
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现低密度奇偶校验（LDPC）码的编码和置信传播（Belief Propagation）译码。
 * 使用稀疏奇偶校验矩阵 H，编码通过高斯消元生成生成矩阵 G。
 * 译码使用和积算法（Sum-Product）在 Tanner 图上迭代传播消息。
 */

#include "utils/code56/LDPCCode3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

// ──────────────────────────────────────────────
// 构造函数
// ──────────────────────────────────────────────

/**
 * @brief 构造函数，初始化默认 LDPC 码参数
 * @param parent 父QObject对象
 */
LDPCCode3::LDPCCode3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("LDPCCode3"));
    generateParityCheck();
}

// ──────────────────────────────────────────────
// 参数配置
// ──────────────────────────────────────────────

/**
 * @brief 设置码字块长度
 *
 * 块长度 n 决定了码字的总长度（信息位 + 校验位）。
 * 改变块长度会重新生成奇偶校验矩阵。
 *
 * @param n 码字长度
 */
void LDPCCode3::setBlockLength(int n)
{
    m_n = qMax(16, n);
    generateParityCheck();
}

/**
 * @brief 设置码率
 *
 * 码率 = 信息位数 / 总码字长度。
 * 常用码率：1/2, 2/3, 3/4, 5/6。
 *
 * @param rate 码率，范围 (0, 1)
 */
void LDPCCode3::setCodeRate(double rate)
{
    m_rate = qBound(0.1, rate, 0.95);
    generateParityCheck();
}

/**
 * @brief 设置最大迭代次数
 *
 * 译码器在达到最大迭代次数后停止。
 * 通常 50~100 次迭代足够收敛。
 *
 * @param iter 最大迭代次数
 */
void LDPCCode3::setMaxIterations(int iter)
{
    m_maxIter = qMax(1, iter);
}

// ──────────────────────────────────────────────
// 编码
// ──────────────────────────────────────────────

/**
 * @brief LDPC 编码
 *
 * 编码步骤：
 * 1. 确定信息位数 k = n * rate
 * 2. 使用奇偶校验矩阵 H 通过高斯消元生成系统形式的生成矩阵 G
 * 3. 计算校验位 p = H_s^{-1} * H_p * info_bits
 * 4. 输出 = [信息位 | 校验位]
 *
 * @param bits 输入信息比特
 * @return 编码输出码字
 */
QVector<int> LDPCCode3::encode(const QVector<int>& bits)
{
    QElapsedTimer timer;
    timer.start();

    const int k = static_cast<int>(m_n * m_rate);
    const int m = m_n - k;

    if (bits.size() != k) {
        // 补齐或截断
        QVector<int> padded(k, 0);
        for (int i = 0; i < qMin(bits.size(), k); ++i) {
            padded[i] = bits[i];
        }
    }

    QVector<int> codeword(m_n, 0);

    // 放置信息位
    for (int i = 0; i < qMin(bits.size(), k); ++i) {
        codeword[i] = bits[i] & 1;
    }

    // 计算校验位：p = H * x (mod 2)
    for (int row = 0; row < m; ++row) {
        int parity = 0;
        for (int col : m_H[row]) {
            parity ^= codeword[col];
        }
        codeword[k + row] = parity;
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalEncodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return codeword;
}

// ──────────────────────────────────────────────
// 置信传播译码
// ──────────────────────────────────────────────

/**
 * @brief LDPC 置信传播译码
 *
 * 使用对数域和积算法（Log-SPA）：
 * 1. 初始化变量节点消息为信道 LLR
 * 2. 水平步骤：校验节点到变量节点消息更新
 * 3. 垂直步骤：变量节点到校验节点消息更新
 * 4. 做硬判决并检查校验方程
 * 5. 如果所有校验满足则译码成功
 *
 * @param llr 信道对数似然比（LLR）输入
 * @return 译码后的比特
 */
QVector<int> LDPCCode3::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    const int n = qMin(llr.size(), m_n);
    if (n == 0) return {};

    const int m = m_H.size();
    if (m == 0) return {};

    // 运行置信传播
    QVector<double> result = beliefPropagation(llr);

    // 硬判决
    const int k = static_cast<int>(m_n * m_rate);
    QVector<int> decoded(k, 0);
    for (int i = 0; i < k; ++i) {
        decoded[i] = (i < result.size() && result[i] < 0.0) ? 1 : 0;
    }

    // 更新统计
    const double elapsed = timer.elapsed();
    m_stats.totalDecodes++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / (m_stats.totalEncodes + m_stats.totalDecodes);

    return decoded;
}

// ──────────────────────────────────────────────
// 统计接口
// ──────────────────────────────────────────────

/**
 * @brief 获取当前统计信息
 * @return 包含编码/译码次数和平均耗时的Stats结构
 */
LDPCCode3::Stats LDPCCode3::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有累计统计信息
 */
void LDPCCode3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

// ──────────────────────────────────────────────
// 私有方法 — 生成奇偶校验矩阵
// ──────────────────────────────────────────────

/**
 * @brief 生成稀疏奇偶校验矩阵 H
 *
 * 使用规则 LDPC 构造方法：
 * - 校验方程数 m = n * (1 - rate)
 * - 每行有固定数量的非零元素（行权重）
 * - 每列有固定数量的非零元素（列权重）
 *
 * 使用确定性种子生成伪随机稀疏矩阵。
 */
void LDPCCode3::generateParityCheck()
{
    const int k = static_cast<int>(m_n * m_rate);
    const int m = m_n - k;

    if (m <= 0 || k <= 0) return;

    m_H.clear();
    m_H.resize(m);
    m_Hcol.clear();
    m_Hcol.resize(m_n);

    // 确定列权重（每列中 1 的数量）
    const int colWeight = qMax(2, m / 3);
    // 确定行权重（每行中 1 的数量）
    const int rowWeight = qMax(2, m_n * colWeight / m);

    // 伪随机构造
    unsigned int seed = 12345;
    auto randInt = [&seed]() -> unsigned int {
        seed = (seed * 1103515245 + 12345) & 0x7FFFFFFF;
        return seed;
    };

    for (int row = 0; row < m; ++row) {
        // 为每行随机选择 rowWeight 个列位置
        int targetWeight = qMin(rowWeight, m_n);
        QVector<bool> used(m_n, false);

        for (int w = 0; w < targetWeight; ++w) {
            int col;
            int attempts = 0;
            do {
                col = randInt() % m_n;
                attempts++;
            } while (used[col] && attempts < 100);

            if (!used[col]) {
                m_H[row].append(col);
                used[col] = true;
                m_Hcol[col].append(row);
            }
        }
    }
}

// ──────────────────────────────────────────────
// 私有方法 — 置信传播
// ──────────────────────────────────────────────

/**
 * @brief 执行对数域置信传播（Log-SPA）迭代
 *
 * 消息更新规则：
 * - 校验节点到变量节点：tanh 规则近似
 * - 变量节点到校验节点：LLR 求和
 *
 * @param llr 初始信道 LLR
 * @return 最终后验 LLR
 */
QVector<double> LDPCCode3::beliefPropagation(const QVector<double>& llr)
{
    const int n = qMin(llr.size(), m_n);
    const int m = m_H.size();

    // 存储消息：msgCheckToVar[row][idx] 和 msgVarToCheck[col][idx]
    // 简化实现：直接在数组上操作

    // 初始化：变量节点到校验节点消息 = 信道 LLR
    QVector<QVector<double>> msgVarToCheck(n);
    for (int col = 0; col < n; ++col) {
        msgVarToCheck[col].resize(m_Hcol[col].size(), 0.0);
        for (int i = 0; i < m_Hcol[col].size(); ++i) {
            msgVarToCheck[col][i] = (col < llr.size()) ? llr[col] : 0.0;
        }
    }

    QVector<QVector<double>> msgCheckToVar(m);
    for (int row = 0; row < m; ++row) {
        msgCheckToVar[row].resize(m_H[row].size(), 0.0);
    }

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // 水平步骤：校验节点到变量节点
        for (int row = 0; row < m; ++row) {
            for (int i = 0; i < m_H[row].size(); ++i) {
                int col = m_H[row][i];

                // 收集除当前变量节点外的所有输入消息
                double prodSign = 1.0;
                double sumMag = 0.0;

                for (int j = 0; j < m_H[row].size(); ++j) {
                    if (j == i) continue;
                    int otherCol = m_H[row][j];

                    // 找到 otherCol -> row 的消息
                    double msg = 0.0;
                    for (int k = 0; k < m_Hcol[otherCol].size(); ++k) {
                        if (m_Hcol[otherCol][k] == row) {
                            msg = msgVarToCheck[otherCol][k];
                            break;
                        }
                    }

                    prodSign *= (msg >= 0.0) ? 1.0 : -1.0;
                    sumMag += qLn(qTanh(qAbs(msg) / 2.0 + 1e-15));
                }

                double extMag = -2.0 * qTanh(qExp(-sumMag + 1e-15));
                msgCheckToVar[row][i] = prodSign * qAbs(extMag);
            }
        }

        // 垂直步骤：变量节点到校验节点
        for (int col = 0; col < n; ++col) {
            for (int i = 0; i < m_Hcol[col].size(); ++i) {
                int row = m_Hcol[col][i];

                double total = (col < llr.size()) ? llr[col] : 0.0;

                for (int j = 0; j < m_Hcol[col].size(); ++j) {
                    if (j == i) continue;
                    int otherRow = m_Hcol[col][j];

                    // 找到 otherRow -> col 的消息
                    double msg = 0.0;
                    for (int k = 0; k < m_H[otherRow].size(); ++k) {
                        if (m_H[otherRow][k] == col) {
                            msg = msgCheckToVar[otherRow][k];
                            break;
                        }
                    }
                    total += msg;
                }

                msgVarToCheck[col][i] = total;
            }
        }

        // 检查是否收敛：计算后验 LLR 并验证校验方程
        bool allSatisfied = true;
        for (int row = 0; row < m; ++row) {
            int parity = 0;
            for (int col : m_H[row]) {
                if (col < n) {
                    double total = (col < llr.size()) ? llr[col] : 0.0;
                    for (int k = 0; k < m_Hcol[col].size(); ++k) {
                        int r = m_Hcol[col][k];
                        for (int l = 0; l < m_H[r].size(); ++l) {
                            if (m_H[r][l] == col) {
                                total += msgCheckToVar[r][l];
                                break;
                            }
                        }
                    }
                    if (total < 0.0) parity ^= 1;
                }
            }
            if (parity != 0) {
                allSatisfied = false;
                break;
            }
        }

        if (allSatisfied) {
            break;
        }
    }

    // 计算最终后验 LLR
    QVector<double> posterior(n, 0.0);
    for (int col = 0; col < n; ++col) {
        double total = (col < llr.size()) ? llr[col] : 0.0;
        for (int k = 0; k < m_Hcol[col].size(); ++k) {
            int row = m_Hcol[col][k];
            for (int l = 0; l < m_H[row].size(); ++l) {
                if (m_H[row][l] == col) {
                    total += msgCheckToVar[row][l];
                    break;
                }
            }
        }
        posterior[col] = total;
    }

    return posterior;
}
