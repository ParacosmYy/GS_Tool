/**
 * @file LdpcDecoder2.cpp
 * @brief LDPC解码器实现 — 置信传播(和积)算法
 */

#include "utils/code8/LdpcDecoder2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <numeric>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
LdpcDecoder2::LdpcDecoder2(QObject* parent)
    : QObject(parent)
{
}

/** @brief 从稀疏矩阵设置校验矩阵H @param rowColPairs 非零位置列表 @param rows 行数 @param cols 列数 */
void LdpcDecoder2::setParityMatrix(const QList<QPair<int, int>>& rowColPairs,
                                    int rows, int cols)
{
    m_m = rows;
    m_n = cols;
    m_k = m_n - m_m;
    m_checkEdges.resize(m_m);
    m_varEdges.resize(m_n);

    for (auto& edges : m_checkEdges) edges.clear();
    for (auto& edges : m_varEdges) edges.clear();

    for (const auto& [row, col] : rowColPairs) {
        if (row >= 0 && row < m_m && col >= 0 && col < m_n) {
            m_checkEdges[row].append(col);
            m_varEdges[col].append(row);
        }
    }

    /* 分配消息矩阵空间 */
    m_R.resize(m_n);
    m_Q.resize(m_m);
    for (int v = 0; v < m_n; ++v) {
        m_R[v].resize(m_varEdges[v].size(), 0.0);
    }
    for (int c = 0; c < m_m; ++c) {
        m_Q[c].resize(m_checkEdges[c].size(), 0.0);
    }
}

/** @brief 设置解码算法 @param algo 算法类型 */
void LdpcDecoder2::setAlgorithm(Algorithm algo)
{
    m_algo = algo;
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void LdpcDecoder2::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(1, maxIter);
}

/** @brief 设置偏移最小和的缩放因子 @param factor 缩放因子 */
void LdpcDecoder2::setMinSumScaleFactor(double factor)
{
    m_minSumFactor = qBound(0.1, factor, 2.0);
}

/** @brief 解码LLR软信息 @param llr 输入LLR @return 解码结果 */
LdpcDecoder2::DecodeResult LdpcDecoder2::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    DecodeResult result;
    if (llr.size() != m_n || m_n == 0) {
        result.converged = false;
        return result;
    }

    /* 初始化: R消息清零，Q消息初始化为信道LLR */
    for (int v = 0; v < m_n; ++v) {
        for (int idx = 0; idx < m_varEdges[v].size(); ++idx) {
            m_R[v][idx] = 0.0;
        }
    }

    /* 迭代解码 */
    bool converged = false;
    int iter = 0;
    for (iter = 0; iter < m_maxIterations; ++iter) {
        /* 变量节点更新: Q(c,v) = LLR(v) + sum(R(v',v)) for v'!=c */
        variableNodeUpdate();

        /* 校验节点更新 */
        if (m_algo == Algorithm::SumProduct) {
            checkNodeUpdateSumProduct(iter);
        } else {
            checkNodeUpdateMinSum();
        }

        /* 硬判决和校验 */
        QVector<int> hardBits(m_n);
        QVector<double> totalLLR(m_n);
        for (int v = 0; v < m_n; ++v) {
            totalLLR[v] = llr[v];
            for (int idx = 0; idx < m_varEdges[v].size(); ++idx) {
                totalLLR[v] += m_R[v][idx];
            }
            hardBits[v] = hardDecision(totalLLR[v]);
        }

        /* 校验 */
        QVector<int> syndrome = computeSyndrome(hardBits);
        bool allZero = true;
        double syndWeight = 0.0;
        for (int s : syndrome) {
            if (s != 0) { allZero = false; }
            syndWeight += static_cast<double>(s);
        }

        if (allZero) {
            converged = true;
            result.decodedBits = hardBits;
            break;
        }

        /* 最后一次迭代也保存结果 */
        if (iter == m_maxIterations - 1) {
            result.decodedBits = hardBits;
        }
    }

    result.converged = converged;
    result.iterations = iter;
    result.finalSyndromeWeight = 0.0;

    /* 估计BER */
    int errorCount = 0;
    for (int v = 0; v < m_n; ++v) {
        if (result.decodedBits[v] != hardDecision(llr[v])) {
            errorCount++;
        }
    }
    result.estimatedBER = static_cast<double>(errorCount) / qMax(1, m_n);

    /* 更新统计 */
    m_stats.totalDecoded++;
    if (converged) m_stats.totalConverged++;
    m_stats.totalIterations += iter;
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalDecoded);
    m_stats.avgIterations = static_cast<double>(m_stats.totalIterations)
        / static_cast<double>(m_stats.totalDecoded);

    emit frameDecoded(m_stats.totalDecoded - 1, converged, iter);
    return result;
}

/** @brief 批量解码 @param llrFrames LLR帧列表 @return 解码结果列表 */
QList<LdpcDecoder2::DecodeResult> LdpcDecoder2::decodeBatch(
    const QList<QVector<double>>& llrFrames)
{
    QList<DecodeResult> results;
    for (int i = 0; i < llrFrames.size(); ++i) {
        results.append(decode(llrFrames[i]));
        emit batchProgress(i + 1, llrFrames.size());
    }
    return results;
}

/** @brief 编码(系统码) @param infoBits 信息比特 @return 码字 */
QVector<int> LdpcDecoder2::encode(const QVector<int>& infoBits) const
{
    if (infoBits.size() != m_k) return {};

    QVector<int> codeword(m_n, 0);
    /* 系统部分 */
    for (int i = 0; i < m_k; ++i) {
        codeword[i] = infoBits[i];
    }

    /* 校验位: p_c = sum(H(c,j) * x_j) mod 2 */
    for (int c = 0; c < m_m; ++c) {
        int parity = 0;
        for (int v : m_checkEdges[c]) {
            parity ^= codeword[v];
        }
        codeword[m_k + c] = parity;
    }
    return codeword;
}

/** @brief 校验码字 @param codeword 码字 @return 是否通过 */
bool LdpcDecoder2::checkSyndrome(const QVector<int>& codeword) const
{
    if (codeword.size() != m_n) return false;
    QVector<int> syndrome = computeSyndrome(codeword);
    for (int s : syndrome) {
        if (s != 0) return false;
    }
    return true;
}

/** @brief 获取码长 @return 码长n */
int LdpcDecoder2::codeLength() const { return m_n; }

/** @brief 获取信息位长度 @return 信息位长度k */
int LdpcDecoder2::infoLength() const { return m_k; }

/** @brief 重置统计 */
void LdpcDecoder2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief 初始化Tanner图消息 */
void LdpcDecoder2::initializeMessages()
{
    for (int v = 0; v < m_n; ++v) {
        std::fill(m_R[v].begin(), m_R[v].end(), 0.0);
    }
}

/** @brief 校验节点更新(和积) */
void LdpcDecoder2::checkNodeUpdateSumProduct(int iteration)
{
    for (int c = 0; c < m_m; ++c) {
        int dc = m_checkEdges[c].size();
        if (dc == 0) continue;

        /* 对每个连接的变量节点v计算R(c,v) */
        for (int idx = 0; idx < dc; ++idx) {
            int v = m_checkEdges[c][idx];

            /* R(c,v) = 2 * atanh(prod(tanh(Q(c,v')/2))) for v'!=v */
            double prodTanH = 1.0;
            for (int j = 0; j < dc; ++j) {
                if (j == idx) continue;
                int v2 = m_checkEdges[c][j];
                /* 找v2在varEdges中的索引以获取Q消息 */
                double qVal = 0.0;
                for (int vi = 0; vi < m_varEdges[v2].size(); ++vi) {
                    if (m_varEdges[v2][vi] == c) {
                        qVal = m_Q[c][j];
                        break;
                    }
                }
                double t = std::tanh(qVal / 2.0);
                t = qBound(-1.0 + 1e-12, t, 1.0 - 1e-12);
                prodTanH *= t;
            }

            prodTanH = qBound(-1.0 + 1e-12, prodTanH, 1.0 - 1e-12);
            double rVal = 2.0 * std::atanh(prodTanH);

            /* 写回R消息 */
            for (int ri = 0; ri < m_varEdges[v].size(); ++ri) {
                if (m_varEdges[v][ri] == c) {
                    m_R[v][ri] = rVal;
                    break;
                }
            }
        }
    }
}

/** @brief 校验节点更新(最小和) */
void LdpcDecoder2::checkNodeUpdateMinSum()
{
    for (int c = 0; c < m_m; ++c) {
        int dc = m_checkEdges[c].size();
        if (dc == 0) continue;

        /* 收集所有Q消息 */
        QVector<double> qMsgs(dc);
        for (int idx = 0; idx < dc; ++idx) {
            qMsgs[idx] = m_Q[c][idx];
        }

        /* 计算最小绝对值和次小绝对值 */
        double minAbs = 1e30, secMinAbs = 1e30;
        int signProd = 1;
        for (int idx = 0; idx < dc; ++idx) {
            double absVal = qAbs(qMsgs[idx]);
            if (absVal < minAbs) {
                secMinAbs = minAbs;
                minAbs = absVal;
            } else if (absVal < secMinAbs) {
                secMinAbs = absVal;
            }
            if (qMsgs[idx] < 0) signProd *= -1;
        }

        /* R(c,v) = sign * min(|others|) * factor */
        for (int idx = 0; idx < dc; ++idx) {
            int v = m_checkEdges[c][idx];
            double absVal = qAbs(qMsgs[idx]);
            double useMin = (absVal == minAbs) ? secMinAbs : minAbs;
            int sign = signProd;
            if (qMsgs[idx] < 0) sign *= -1;

            double rVal = sign * useMin * m_minSumFactor;

            for (int ri = 0; ri < m_varEdges[v].size(); ++ri) {
                if (m_varEdges[v][ri] == c) {
                    m_R[v][ri] = rVal;
                    break;
                }
            }
        }
    }
}

/** @brief 变量节点更新 */
void LdpcDecoder2::variableNodeUpdate()
{
    /* Q(c,v) = LLR(v) + sum(R(c',v)) for c'!=c */
    /* 我们先存入临时数组 */
    for (int v = 0; v < m_n; ++v) {
        int dv = m_varEdges[v].size();
        /* totalLLR[v]会在decode()中处理 */
        for (int idx = 0; idx < dv; ++idx) {
            int c = m_varEdges[v][idx];
            /* Q(c,v) = channelLLR + sum(R except from c) */
            /* 需要找到对应的checkEdges位置 */
            for (int ci = 0; ci < m_checkEdges[c].size(); ++ci) {
                if (m_checkEdges[c][ci] == v) {
                    m_Q[c][ci] = m_R[v][idx];
                    break;
                }
            }
        }
    }
}

/** @brief 硬判决 */
int LdpcDecoder2::hardDecision(double llr) const
{
    return (llr < 0.0) ? 1 : 0;
}

/** @brief 计算校验子 */
QVector<int> LdpcDecoder2::computeSyndrome(const QVector<int>& bits) const
{
    QVector<int> syndrome(m_m, 0);
    for (int c = 0; c < m_m; ++c) {
        int sum = 0;
        for (int v : m_checkEdges[c]) {
            sum ^= bits[v];
        }
        syndrome[c] = sum;
    }
    return syndrome;
}

/** @brief phi函数: ln(tanh(|x|/2)) */
double LdpcDecoder2::phiFunction(double x) const
{
    if (x < 1e-12) return 30.0; /* 避免log(0) */
    double t = std::tanh(x / 2.0);
    if (t < 1e-30) return 30.0;
    return qLn(t);
}
