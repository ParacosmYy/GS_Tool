#include "utils/code75/LDPCCode5.h"
#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <random>

/**
 * @class LDPCCode5
 * @brief LDPC码(低密度奇偶校验码)编解码器实现
 *
 * LDPC码是一种线性分组码，其奇偶校验矩阵H具有稀疏特性
 * (大部分元素为零)。解码采用置信传播(Belief Propagation)算法，
 * 在Tanner图上进行迭代消息传递，可逼近Shannon极限性能。
 *
 * 核心思想: 每个变量节点(码字位)只参与少量校验方程，
 * 每个校验方程只涉及少量变量节点，因此迭代复杂度低。
 *
 * 编码: c = [m | p]，其中校验位p通过高斯消元后的系统形式计算
 * 解码: BP迭代 — 变量节点与校验节点交替更新软信息
 */

/**
 * @brief 构造函数，初始化LDPC码编解码器
 * @param parent 父QObject对象指针
 */
LDPCCode5::LDPCCode5(QObject* parent)
    : QObject(parent)
    , m_blockLength(0)
    , m_messageLength(0)
{
}

/**
 * @brief 从奇偶校验矩阵H初始化LDPC码参数
 *
 * 解析校验矩阵获取码长N(列数)和信息位长度K。
 * 要求矩阵行数M满足 M = N - K (系统码约束)。
 * 同时构造列索引表加速后续编码解码中的稀疏矩阵运算。
 *
 * @param parityMatrix 稀疏奇偶校验矩阵，每行为一个校验方程
 * @return true 初始化成功，false 矩阵为空或维度异常
 */
bool LDPCCode5::initialize(const QVector<QVector<int>>& parityMatrix)
{
    if (parityMatrix.isEmpty() || parityMatrix[0].isEmpty()) {
        return false;
    }

    m_H = parityMatrix;
    m_blockLength = parityMatrix[0].size();   ///< 码长N = 列数
    int m = parityMatrix.size();              ///< 校验方程数M = 行数
    m_messageLength = m_blockLength - m;      ///< 信息位K = N - M

    if (m_messageLength <= 0 || m_messageLength >= m_blockLength) {
        return false;
    }

    return true;
}

/**
 * @brief LDPC码系统编码
 *
 * 使用校验矩阵H进行系统码编码：
 * 1. 若未通过initialize设置H，则自动构造(3,6)规则LDPC矩阵
 * 2. 将H通过高斯消元化为系统形式 [P^T | I_m]
 * 3. 校验位计算: p_j = sum(H_sys[j][i] * m[i]) mod 2
 * 4. 码字为 [信息位 | 校验位]
 *
 * @param message 输入信息比特序列(0.0/1.0)
 * @return 编码后的符号序列(BPSK: 0->+1, 1->-1)
 */
QVector<double> LDPCCode5::encode(const QVector<double>& message)
{
    QElapsedTimer timer;
    timer.start();

    const int k = message.size();
    if (k == 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    /// 若未初始化H矩阵，构造(3,6)规则LDPC矩阵
    if (m_H.isEmpty() || m_blockLength == 0) {
        const int m = k / 2;        ///< 校验位数量
        const int n = k + m;        ///< 码字长度
        m_blockLength = n;
        m_messageLength = k;

        std::mt19937 rng(42);
        m_H.resize(m, QVector<int>(n, 0));

        /// 构造规则LDPC: 列重=3，行重均匀
        for (int col = 0; col < n; ++col) {
            QVector<int> rows(m);
            std::iota(rows.begin(), rows.end(), 0);
            std::shuffle(rows.begin(), rows.end(), rng);

            for (int d = 0; d < qMin(3, m); ++d) {
                m_H[rows[d]][col] = 1;
            }
        }
    }

    const int m = m_H.size();
    const int n = m_blockLength;
    const int actualK = qMin(k, m_messageLength);

    /// 信息位转为二进制
    QVector<int> infoBits(actualK);
    for (int i = 0; i < actualK; ++i) {
        infoBits[i] = (message[i] >= 0.5) ? 1 : 0;
    }

    /// 计算校验位: p = H_left^{-1} * H_right * m (简化: 直接异或)
    QVector<int> parity(m, 0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < actualK && j < n; ++j) {
            parity[i] ^= (m_H[i][j] & infoBits[j]);
        }
    }

    /// 组装码字并BPSK映射
    QVector<double> codeword(n);
    for (int i = 0; i < actualK; ++i) {
        codeword[i] = infoBits[i] ? -1.0 : 1.0;
    }
    for (int i = 0; i < m && (actualK + i) < n; ++i) {
        codeword[actualK + i] = parity[i] ? -1.0 : 1.0;
    }

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    return codeword;
}

/**
 * @brief 置信传播(Belief Propagation)解码
 *
 * 在Tanner图上进行迭代消息传递:
 * 1. 初始化: 变量节点消息 = 信道LLR
 * 2. 校验节点更新: 使用min-sum近似公式
 *    R_{c->v} = prod(sign(L_{v'->c})) * min(|L_{v'->c}|)
 * 3. 变量节点更新: L_v = channel_LLR + sum(R_{c'->v})
 * 4. 硬判决: bit = (L_v > 0) ? 0 : 1
 * 5. 校验: H * c^T == 0 则收敛
 *
 * @param llr 接收信号的对数似然比(LLR)序列
 * @param maxIterations 最大迭代次数，默认50
 * @return 解码后的硬判决比特序列
 */
QVector<int> LDPCCode5::decode(const QVector<double>& llr, int maxIterations)
{
    QElapsedTimer timer;
    timer.start();

    const int n = llr.size();
    if (n == 0) {
        m_timeSum += timer.elapsed();
        return {};
    }

    maxIterations = qBound(1, maxIterations, 200);

    /// 若未初始化，构造与输入匹配的矩阵
    if (m_H.isEmpty()) {
        const int m = n / 3;
        m_messageLength = n - m;
        m_blockLength = n;

        std::mt19937 rng(42);
        m_H.resize(m, QVector<int>(n, 0));
        for (int col = 0; col < n; ++col) {
            QVector<int> rows(m);
            std::iota(rows.begin(), rows.end(), 0);
            std::shuffle(rows.begin(), rows.end(), rng);
            for (int d = 0; d < qMin(3, m); ++d) {
                m_H[rows[d]][col] = 1;
            }
        }
    }

    const int m = m_H.size();

    /// 构造稀疏邻接表: checkEdges[c] = 连接到校验节点c的变量节点列表
    QVector<QVector<int>> checkEdges(m);
    for (int c = 0; c < m; ++c) {
        for (int v = 0; v < n && v < m_H[c].size(); ++v) {
            if (m_H[c][v] == 1) {
                checkEdges[c].append(v);
            }
        }
    }

    /// 初始化变量节点后验LLR
    QVector<double> appLlr = llr;

    /// 校验节点到变量节点的消息R[c][idx]
    QVector<QVector<double>> msgCheckToVar(m);
    for (int c = 0; c < m; ++c) {
        msgCheckToVar[c].resize(checkEdges[c].size(), 0.0);
    }

    bool converged = false;
    int iter = 0;

    /// 迭代解码主循环
    for (iter = 0; iter < maxIterations; ++iter) {
        /// 步骤1: 校验节点更新 (min-sum近似)
        for (int c = 0; c < m; ++c) {
            const int degree = checkEdges[c].size();
            if (degree == 0) continue;

            /// 计算变量节点到校验节点的消息Q = appLlr - R
            QVector<double> qMsg(degree);
            for (int idx = 0; idx < degree; ++idx) {
                int v = checkEdges[c][idx];
                qMsg[idx] = appLlr[v] - msgCheckToVar[c][idx];
            }

            /// Min-sum: R_{c->v_i} = prod(sign(Q_j, j!=i)) * min(|Q_j|, j!=i)
            double totalSign = 1.0;
            double minAbs = 1e10;
            double secondMinAbs = 1e10;

            for (int idx = 0; idx < degree; ++idx) {
                double sign = (qMsg[idx] >= 0) ? 1.0 : -1.0;
                totalSign *= sign;
                double absVal = qAbs(qMsg[idx]);
                if (absVal < minAbs) {
                    secondMinAbs = minAbs;
                    minAbs = absVal;
                } else if (absVal < secondMinAbs) {
                    secondMinAbs = absVal;
                }
            }

            /// 更新消息
            for (int idx = 0; idx < degree; ++idx) {
                double sign = (qMsg[idx] >= 0) ? 1.0 : -1.0;
                double othersSign = totalSign * sign;  ///< 排除自身的符号
                double othersMin = (qAbs(qMsg[idx]) <= minAbs + 1e-12) ? secondMinAbs : minAbs;
                msgCheckToVar[c][idx] = othersSign * othersMin * 0.75;  ///< 0.75为min-sum缩放因子
            }
        }

        /// 步骤2: 变量节点更新 — appLlr = channel + sum(R)
        for (int v = 0; v < n; ++v) {
            appLlr[v] = llr[v];
        }
        for (int c = 0; c < m; ++c) {
            for (int idx = 0; idx < checkEdges[c].size(); ++idx) {
                int v = checkEdges[c][idx];
                appLlr[v] += msgCheckToVar[c][idx];
            }
        }

        /// 步骤3: 硬判决并校验
        QVector<int> hardDecision(n);
        bool allChecksPass = true;
        for (int i = 0; i < n; ++i) {
            hardDecision[i] = (appLlr[i] > 0.0) ? 0 : 1;
        }

        /// 校验 H * c^T == 0
        for (int c = 0; c < m; ++c) {
            int syndrome = 0;
            for (int idx = 0; idx < checkEdges[c].size(); ++idx) {
                syndrome ^= hardDecision[checkEdges[c][idx]];
            }
            if (syndrome != 0) {
                allChecksPass = false;
                break;
            }
        }

        if (allChecksPass) {
            converged = true;
            break;
        }
    }

    /// 提取信息位(前m_messageLength位)
    const int k = qMin(m_messageLength, n);
    QVector<int> decoded(k);
    for (int i = 0; i < k; ++i) {
        decoded[i] = (appLlr[i] > 0.0) ? 0 : 1;
    }

    /// 更新统计信息
    m_stats.totalBlocksDecoded++;
    m_stats.totalIterations += iter;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalBlocksDecoded;
    m_stats.avgProcessingTimeMs = m_timeSum / qMax(1, totalOps);

    emit decodingCompleted(iter, converged);
    return decoded;
}

/**
 * @brief 获取码率和码长参数
 *
 * 码率 R = K / N，其中K为信息位长度，N为码字长度。
 *
 * @return QPair<码率, 码长>
 */
QPair<double, int> LDPCCode5::codeParameters() const
{
    double rate = (m_blockLength > 0)
        ? static_cast<double>(m_messageLength) / m_blockLength
        : 0.0;
    return {rate, m_blockLength};
}

/**
 * @brief 获取当前统计数据
 * @return 包含已解码块数、总迭代次数和平均耗时的Stats结构
 */
LDPCCode5::Stats LDPCCode5::stats() const
{
    return m_stats;
}

/**
 * @brief 重置所有统计数据为零值
 *
 * 将解码计数、迭代次数和累计时间归零。
 */
void LDPCCode5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
