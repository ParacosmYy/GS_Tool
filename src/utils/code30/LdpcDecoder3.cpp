/**
 * @file LdpcDecoder3.cpp
 * @brief LDPC解码增强实现 — 最小和/分层调度/早期终止/量化消息
 */

#include "utils/code30/LdpcDecoder3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/** @brief 构造函数 @param parent 父对象 */
LdpcDecoder3::LdpcDecoder3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 设置校验矩阵 @param H 稀疏校验矩阵 */
void LdpcDecoder3::setParityMatrix(const QVector<QVector<int>>& H)
{
    m_H = H;
    m_m = H.size();
    m_n = (m_m > 0) ? H[0].size() : 0;
}

/** @brief 设置最大迭代次数 @param maxIter 最大迭代 */
void LdpcDecoder3::setMaxIterations(int maxIter)
{
    m_maxIter = qMax(1, maxIter);
}

/** @brief 设置最小和缩放因子 @param factor 缩放因子 */
void LdpcDecoder3::setScalingFactor(double factor)
{
    m_scaling = qBound(0.1, factor, 1.0);
}

/** @brief 设置早期终止 @param enable 是否启用 */
void LdpcDecoder3::setEarlyTermination(bool enable)
{
    m_earlyStop = enable;
}

/** @brief 标准置信传播解码 @param llr 信道LLR @return 硬判决结果 */
QVector<int> LdpcDecoder3::decode(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    if (m_H.isEmpty() || llr.size() != m_n) {
        return QVector<int>(llr.size(), 0);
    }

    /* 初始化变量节点的后验信息 */
    QVector<double> appL = llr;

    /* 初始化校验节点到变量节点的消息 */
    QVector<QVector<double>> rMsg(m_m, QVector<double>(m_n, 0.0));

    m_lastIter = 0;
    bool converged = false;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        m_lastIter = iter + 1;

        /* 校验节点更新(和积算法) */
        for (int i = 0; i < m_m; ++i) {
            /* 收集所有连接的变量节点消息 */
            double product = 1.0;
            for (int j = 0; j < m_n; ++j) {
                if (m_H[i][j]) {
                    double msg = appL[j] - rMsg[i][j];
                    product *= qTanh(qBound(-30.0, msg * 0.5, 30.0));
                }
            }

            for (int j = 0; j < m_n; ++j) {
                if (m_H[i][j]) {
                    double otherProd = product;
                    double selfMsg = appL[j] - rMsg[i][j];
                    double selfTan = qTanh(qBound(-30.0, selfMsg * 0.5, 30.0));
                    if (qFabs(selfTan) > 1e-12) {
                        otherProd /= selfTan;
                    }
                    otherProd = qBound(-1.0 + 1e-10, otherProd, 1.0 - 1e-10);
                    rMsg[i][j] = 2.0 * qAtanh(otherProd);
                }
            }
        }

        /* 变量节点更新 */
        for (int j = 0; j < m_n; ++j) {
            appL[j] = llr[j];
            for (int i = 0; i < m_m; ++i) {
                if (m_H[i][j]) {
                    appL[j] += rMsg[i][j];
                }
            }
        }

        /* 早期终止: 检查校验方程 */
        if (m_earlyStop) {
            bool allSatisfied = true;
            for (int i = 0; i < m_m; ++i) {
                int parity = 0;
                for (int j = 0; j < m_n; ++j) {
                    if (m_H[i][j] && appL[j] < 0) {
                        parity ^= 1;
                    }
                }
                if (parity != 0) {
                    allSatisfied = false;
                    break;
                }
            }
            if (allSatisfied) {
                converged = true;
                break;
            }
        }
    }

    /* 硬判决 */
    QVector<int> result(m_n);
    for (int j = 0; j < m_n; ++j) {
        result[j] = (appL[j] < 0) ? 1 : 0;
    }

    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += m_n;
    m_stats.avgIterations = (m_stats.avgIterations * (m_stats.totalDecodes - 1)
        + m_lastIter) / m_stats.totalDecodes;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalDecodes));

    emit decodeComplete(m_lastIter, converged);
    return result;
}

/** @brief 最小和解码 @param llr 信道LLR @return 硬判决结果 */
QVector<int> LdpcDecoder3::decodeMinSum(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    if (m_H.isEmpty() || llr.size() != m_n) {
        return QVector<int>(llr.size(), 0);
    }

    QVector<double> appL = llr;
    QVector<QVector<double>> rMsg(m_m, QVector<double>(m_n, 0.0));

    m_lastIter = 0;
    bool converged = false;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        m_lastIter = iter + 1;

        for (int i = 0; i < m_m; ++i) {
            /* 收集连接变量节点的消息 */
            for (int j = 0; j < m_n; ++j) {
                if (!m_H[i][j]) continue;

                double minMag = std::numeric_limits<double>::max();
                int signProd = 1;

                for (int jj = 0; jj < m_n; ++jj) {
                    if (jj == j || !m_H[i][jj]) continue;
                    double msg = appL[jj] - rMsg[i][jj];
                    double mag = qFabs(msg);
                    if (mag < minMag) minMag = mag;
                    if (msg < 0) signProd *= -1;
                }

                rMsg[i][j] = m_scaling * signProd * minMag;
            }
        }

        for (int j = 0; j < m_n; ++j) {
            appL[j] = llr[j];
            for (int i = 0; i < m_m; ++i) {
                if (m_H[i][j]) {
                    appL[j] += rMsg[i][j];
                }
            }
        }

        if (m_earlyStop) {
            bool ok = true;
            for (int i = 0; i < m_m && ok; ++i) {
                int p = 0;
                for (int j = 0; j < m_n; ++j) {
                    if (m_H[i][j] && appL[j] < 0) p ^= 1;
                }
                if (p != 0) ok = false;
            }
            if (ok) { converged = true; break; }
        }
    }

    QVector<int> result(m_n);
    for (int j = 0; j < m_n; ++j) {
        result[j] = (appL[j] < 0) ? 1 : 0;
    }

    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalDecodes));

    emit decodeComplete(m_lastIter, converged);
    return result;
}

/** @brief 分层调度解码(逐行更新加速收敛) @param llr 信道LLR @return 硬判决结果 */
QVector<int> LdpcDecoder3::decodeLayered(const QVector<double>& llr)
{
    QElapsedTimer timer;
    timer.start();

    if (m_H.isEmpty() || llr.size() != m_n) {
        return QVector<int>(llr.size(), 0);
    }

    /* 分层解码: 逐层(行)更新, 收敛速度更快 */
    QVector<double> appL = llr;
    QVector<QVector<double>> rMsg(m_m, QVector<double>(m_n, 0.0));

    m_lastIter = 0;
    bool converged = false;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        m_lastIter = iter + 1;

        for (int i = 0; i < m_m; ++i) {
            /* 对第i层(第i个校验方程)的所有变量节点 */
            for (int j = 0; j < m_n; ++j) {
                if (!m_H[i][j]) continue;

                /* 移除旧的外信息 */
                double extrinsic = appL[j] - rMsg[i][j];

                /* 最小和计算 */
                double minMag = std::numeric_limits<double>::max();
                int signProd = 1;

                for (int jj = 0; jj < m_n; ++jj) {
                    if (jj == j || !m_H[i][jj]) continue;
                    double msg = appL[jj] - rMsg[i][jj];
                    double mag = qFabs(msg);
                    if (mag < minMag) minMag = mag;
                    if (msg < 0) signProd *= -1;
                }

                double newMsg = m_scaling * signProd * minMag;
                /* 立即更新appL */
                appL[j] = extrinsic + newMsg;
                rMsg[i][j] = newMsg;
            }
        }

        /* 早期终止 */
        if (m_earlyStop) {
            bool ok = true;
            for (int i = 0; i < m_m && ok; ++i) {
                int p = 0;
                for (int j = 0; j < m_n; ++j) {
                    if (m_H[i][j] && appL[j] < 0) p ^= 1;
                }
                if (p != 0) ok = false;
            }
            if (ok) { converged = true; break; }
        }
    }

    QVector<int> result(m_n);
    for (int j = 0; j < m_n; ++j) {
        result[j] = (appL[j] < 0) ? 1 : 0;
    }

    m_stats.totalDecodes++;
    m_stats.totalBitsProcessed += m_n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalDecodes));

    emit decodeComplete(m_lastIter, converged);
    return result;
}

/** @brief 获取上次解码的迭代次数 @return 迭代次数 */
int LdpcDecoder3::lastIterations() const
{
    return m_lastIter;
}

/** @brief 重置统计 */
void LdpcDecoder3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
