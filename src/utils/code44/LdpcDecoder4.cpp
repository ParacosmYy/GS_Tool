/**
 * @file LdpcDecoder4.cpp
 * @brief LDPC解码器4实现 — 分层置信传播+早期终止
 *
 * 实现分层置信传播(Layered BP)解码算法，相比标准 flooding BP
 * 收敛速度快约2倍。支持早期终止以降低平均迭代次数。
 */

#include "utils/code44/LdpcDecoder4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject
 */
LdpcDecoder4::LdpcDecoder4(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置校验矩阵H
 * @param H 二进制校验矩阵，H[i][j]=1表示第i个校验方程连接第j个变量节点
 */
void LdpcDecoder4::setParityMatrix(const QVector<QVector<int>>& H)
{
    m_H = H;
    m_n = (H.isEmpty()) ? 0 : H[0].size();
    m_k = m_n - H.size();
    buildGraph();
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大迭代次数，默认50
 */
void LdpcDecoder4::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(1, maxIter);
}

/**
 * @brief 标准flooding置信传播解码
 * @param llr 输入对数似然比(LLR)向量
 * @return 硬判决后的码字向量
 *
 * 标准BP: 所有校验节点并行更新，再更新变量节点。
 */
QVector<int> LdpcDecoder4::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    const int n = llr.size();
    if (n != m_n || m_H.isEmpty()) {
        return QVector<int>(n, 0);
    }

    const int m = m_H.size();

    /* 初始化后验概率 */
    QVector<double> appLlr = llr;

    /* 初始化校验节点到变量节点的消息 */
    QVector<QVector<double>> c2v(m, QVector<double>(n, 0.0));
    QVector<QVector<double>> v2c(n, QVector<double>(m, 0.0));

    /* 初始化v2c为先验信息 */
    for (int j = 0; j < n; ++j) {
        for (int i = 0; i < m; ++i) {
            if (m_H[i][j] == 1) {
                v2c[j][i] = llr[j];
            }
        }
    }

    bool converged = false;
    int iter = 0;

    for (iter = 0; iter < m_maxIterations; ++iter) {
        /* ---- 校验节点更新: tanh规则 ---- */
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                if (m_H[i][j] != 1) continue;

                double prodTanh = 1.0;
                for (int jj = 0; jj < n; ++jj) {
                    if (jj == j || m_H[i][jj] != 1) continue;
                    double t = std::tanh(v2c[jj][i] / 2.0);
                    prodTanh *= t;
                }
                /* clamp避免数值溢出 */
                prodTanh = qBound(-1.0 + 1e-10, prodTanh, 1.0 - 1e-10);
                c2v[i][j] = 2.0 * std::atanh(prodTanh);
            }
        }

        /* ---- 变量节点更新 ---- */
        for (int j = 0; j < n; ++j) {
            double sum = llr[j];
            for (int i = 0; i < m; ++i) {
                if (m_H[i][j] == 1) {
                    sum += c2v[i][j];
                }
            }
            appLlr[j] = sum;

            for (int i = 0; i < m; ++i) {
                if (m_H[i][j] == 1) {
                    v2c[j][i] = sum - c2v[i][j];
                }
            }
        }

        /* ---- 早期终止: 校验校验方程 ---- */
        QVector<int> hardDecision(n, 0);
        for (int j = 0; j < n; ++j) {
            hardDecision[j] = (appLlr[j] < 0) ? 1 : 0;
        }
        if (checkSyndrome(hardDecision)) {
            converged = true;
            break;
        }
    }

    /* 硬判决输出 */
    QVector<int> codeword(n, 0);
    for (int j = 0; j < n; ++j) {
        codeword[j] = (appLlr[j] < 0) ? 1 : 0;
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodes++;
    m_stats.totalIterations += iter;
    m_stats.totalCodewords++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodes;

    emit decodeCompleted(iter, converged);
    return codeword;
}

/**
 * @brief 分层置信传播解码
 * @param llr 输入对数似然比(LLR)向量
 * @return 硬判决后的码字向量
 *
 * 分层BP: 逐层(逐校验方程)更新，收敛速度约为标准BP的2倍。
 * 每更新一个校验层后立即更新对应变量节点的后验概率。
 */
QVector<int> LdpcDecoder4::decodeLayered(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    const int n = llr.size();
    if (n != m_n || m_H.isEmpty()) {
        return QVector<int>(n, 0);
    }

    const int m = m_H.size();

    /* 后验LLR初始化为先验 */
    QVector<double> appLlr = llr;

    /* 残余消息初始化为0 */
    QVector<QVector<double>> residual(m, QVector<double>(n, 0.0));

    bool converged = false;
    int iter = 0;

    for (iter = 0; iter < m_maxIterations; ++iter) {
        /* 逐层处理每个校验方程 */
        for (int i = 0; i < m; ++i) {
            /* 收集当前层所有连接的变量节点 */
            QVector<int> connected;
            for (int j = 0; j < n; ++j) {
                if (m_H[i][j] == 1) {
                    connected.append(j);
                }
            }

            if (connected.isEmpty()) continue;

            /* 计算校验节点到变量节点的外信息 */
            for (int idx = 0; idx < connected.size(); ++idx) {
                int j = connected[idx];

                /* 当前变量节点的有效LLR */
                double currentLlr = appLlr[j] - residual[i][j];

                /* 计算tanh积 */
                double prodTanh = 1.0;
                for (int idx2 = 0; idx2 < connected.size(); ++idx2) {
                    if (idx2 == idx) continue;
                    int jj = connected[idx2];
                    double effLlr = appLlr[jj] - residual[i][jj];
                    double t = std::tanh(effLlr / 2.0);
                    prodTanh *= t;
                }
                prodTanh = qBound(-1.0 + 1e-10, prodTanh, 1.0 - 1e-10);

                double newResidual = 2.0 * std::atanh(prodTanh);
                double delta = newResidual - residual[i][j];

                /* 更新后验LLR和残余 */
                appLlr[j] += delta;
                residual[i][j] = newResidual;
            }
        }

        /* 早期终止检查 */
        QVector<int> hardDecision(n, 0);
        for (int j = 0; j < n; ++j) {
            hardDecision[j] = (appLlr[j] < 0) ? 1 : 0;
        }
        if (checkSyndrome(hardDecision)) {
            converged = true;
            break;
        }
    }

    /* 硬判决输出 */
    QVector<int> codeword(n, 0);
    for (int j = 0; j < n; ++j) {
        codeword[j] = (appLlr[j] < 0) ? 1 : 0;
    }

    /* 更新统计 */
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecodes++;
    m_stats.totalIterations += iter;
    m_stats.totalCodewords++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecodes;

    emit decodeCompleted(iter, converged);
    return codeword;
}

/**
 * @brief 校验码字是否满足所有校验方程
 * @param codeword 待检查的码字
 * @return true表示码字合法（所有校验方程满足），false表示存在错误
 */
bool LdpcDecoder4::checkSyndrome(const QVector<int>& codeword) const
{
    if (m_H.isEmpty()) return false;

    for (int i = 0; i < m_H.size(); ++i) {
        int syndrome = 0;
        for (int j = 0; j < codeword.size() && j < m_H[i].size(); ++j) {
            if (m_H[i][j] == 1) {
                syndrome ^= codeword[j];
            }
        }
        if (syndrome != 0) return false;
    }
    return true;
}

/**
 * @brief 构建Tanner图的连接关系
 *
 * 建立校验节点到变量节点和变量节点到校验节点的邻接表。
 */
void LdpcDecoder4::buildGraph()
{
    if (m_H.isEmpty()) return;

    const int m = m_H.size();
    const int n = m_H[0].size();

    m_checkToVar.resize(m);
    m_varToCheck.resize(n);

    for (int i = 0; i < m; ++i) {
        m_checkToVar[i].clear();
        for (int j = 0; j < n; ++j) {
            if (m_H[i][j] == 1) {
                m_checkToVar[i].append(j);
            }
        }
    }

    for (int j = 0; j < n; ++j) {
        m_varToCheck[j].clear();
        for (int i = 0; i < m; ++i) {
            if (j < m_H[i].size() && m_H[i][j] == 1) {
                m_varToCheck[j].append(i);
            }
        }
    }
}

/**
 * @brief 重置所有统计信息
 */
void LdpcDecoder4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
