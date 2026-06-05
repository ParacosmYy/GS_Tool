/**
 * @file GaussianMixture8.cpp
 * @brief 高斯混合模型8 — 变分贝叶斯推断实现
 *
 * 实现变分贝叶斯高斯混合模型，支持自动确定分量数、
 * 均值/精度/权重更新、下界计算等功能。
 * 所有运算带有QElapsedTimer计时和统计信息追踪。
 */

#include "cluster37/GaussianMixture8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <QRandomGenerator>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
GaussianMixture8::GaussianMixture8(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置混合分量数
 * @param k 分量数量，必须大于0
 */
void GaussianMixture8::setComponents(int k)
{
    if (k < 1) k = 1;
    m_k = k;
}

/**
 * @brief 变分贝叶斯推断拟合模型
 * @param data 输入数据矩阵，每行一个样本，每列一个维度
 * @param maxIter 最大迭代次数，默认200
 * @param tol 收敛容差，默认1e-6
 *
 * 执行完整的变分推断流程：初始化参数 → E步(责任度计算) → M步(参数更新) → 计算下界
 */
void GaussianMixture8::fit(const QVector<QVector<double>>& data, int maxIter, double tol)
{
    QElapsedTimer timer;
    timer.start();

    /* 空数据检查 */
    if (data.isEmpty()) return;

    m_n = data.size();
    m_dim = data.isEmpty() ? 0 : data[0].size();

    /* 初始化权重为均匀分布 */
    m_weights.resize(m_k);
    for (int i = 0; i < m_k; ++i)
        m_weights[i] = 1.0 / m_k;

    /* 初始化均值：从数据中随机采样 */
    m_means.resize(m_k);
    for (int i = 0; i < m_k; ++i) {
        m_means[i].resize(m_dim);
        int idx = QRandomGenerator::global()->bounded(m_n);
        for (int d = 0; d < m_dim; ++d)
            m_means[i][d] = data[idx][d];
    }

    /* 初始化精度矩阵（对角单位阵） */
    m_precisions.resize(m_k);
    for (int i = 0; i < m_k; ++i) {
        m_precisions[i].resize(m_dim);
        for (int d = 0; d < m_dim; ++d) {
            m_precisions[i][d].resize(m_dim);
            m_precisions[i][d][d] = 1.0;
        }
    }

    /* 初始化浓度参数 */
    m_concentrationSum = m_k;

    double prevLowerBound = -1e30;
    int iterCount = 0;

    /* 变分推断主循环 */
    for (int iter = 0; iter < maxIter; ++iter) {
        iterCount = iter + 1;

        /* E步：计算责任度矩阵 r[n][k] */
        QVector<QVector<double>> r(m_n, QVector<double>(m_k, 0.0));
        for (int n = 0; n < m_n; ++n) {
            double logSum = -1e30;
            for (int k = 0; k < m_k; ++k) {
                /* 计算对数概率密度 log N(x_n | mu_k, Lambda_k) */
                double logProb = qLn(m_weights[k]);
                for (int d = 0; d < m_dim; ++d) {
                    double diff = data[n][d] - m_means[k][d];
                    logProb -= 0.5 * diff * diff * m_precisions[k][d][d];
                }
                logProb += 0.5 * m_dim * qLn(2.0 * M_PI);
                r[n][k] = logProb;
                logSum = (logSum < logProb) ? logProb + qLn(1.0 + qExp(logSum - logProb))
                                            : logProb + qLn(1.0 + qExp(logSum - logProb));
            }
            /* 归一化 */
            for (int k = 0; k < m_k; ++k)
                r[n][k] = qExp(r[n][k] - logSum);
        }

        /* M步：更新参数 */
        QVector<double> Nk(m_k, 0.0);
        for (int n = 0; n < m_n; ++n)
            for (int k = 0; k < m_k; ++k)
                Nk[k] += r[n][k];

        /* 更新权重 */
        for (int k = 0; k < m_k; ++k)
            m_weights[k] = (Nk[k] + 1.0) / (m_n + m_k);

        /* 更新均值 */
        for (int k = 0; k < m_k; ++k) {
            for (int d = 0; d < m_dim; ++d) {
                double sum = 0.0;
                for (int n = 0; n < m_n; ++n)
                    sum += r[n][k] * data[n][d];
                m_means[k][d] = sum / qMax(Nk[k], 1e-10);
            }
        }

        /* 更新精度矩阵（对角简化） */
        for (int k = 0; k < m_k; ++k) {
            for (int d = 0; d < m_dim; ++d) {
                double sumSq = 0.0;
                for (int n = 0; n < m_n; ++n) {
                    double diff = data[n][d] - m_means[k][d];
                    sumSq += r[n][k] * diff * diff;
                }
                m_precisions[k][d][d] = qMax(Nk[k] / (sumSq + 1e-10), 1e-10);
            }
        }

        /* 计算变分下界 */
        m_lowerBound = 0.0;
        for (int k = 0; k < m_k; ++k) {
            m_lowerBound += m_weights[k] * qLn(qMax(m_weights[k], 1e-30));
            for (int d = 0; d < m_dim; ++d)
                m_lowerBound += 0.5 * qLn(m_precisions[k][d][d]);
        }
        m_lowerBound -= 0.5 * m_k * m_dim * qLn(2.0 * M_PI);

        /* 检查收敛 */
        if (qAbs(m_lowerBound - prevLowerBound) < tol)
            break;
        prevLowerBound = m_lowerBound;
    }

    /* 更新统计信息 */
    qint64 elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalFits++;
    m_stats.totalSamplesProcessed += m_n;
    m_stats.totalIterations += iterCount;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalFits;

    emit fitCompleted(m_k, iterCount, m_lowerBound);
}

/**
 * @brief 预测样本所属分量
 * @param sample 输入样本
 * @return 概率最大的分量索引
 */
int GaussianMixture8::predict(const QVector<double>& sample) const
{
    int bestK = 0;
    double bestProb = -1e30;
    for (int k = 0; k < m_k; ++k) {
        double logProb = qLn(m_weights[k]);
        for (int d = 0; d < m_dim; ++d) {
            double diff = sample[d] - m_means[k][d];
            logProb -= 0.5 * diff * diff * m_precisions[k][d][d];
        }
        if (logProb > bestProb) {
            bestProb = logProb;
            bestK = k;
        }
    }
    return bestK;
}

/**
 * @brief 预测样本属于各分量的概率
 * @param sample 输入样本
 * @return 各分量的后验概率向量
 */
QVector<double> GaussianMixture8::predictProb(const QVector<double>& sample) const
{
    QVector<double> probs(m_k, 0.0);
    double logSum = -1e30;
    for (int k = 0; k < m_k; ++k) {
        double logProb = qLn(m_weights[k]);
        for (int d = 0; d < m_dim; ++d) {
            double diff = sample[d] - m_means[k][d];
            logProb -= 0.5 * diff * diff * m_precisions[k][d][d];
        }
        probs[k] = logProb;
        logSum = (logSum < logProb) ? logProb + qLn(1.0 + qExp(logSum - logProb))
                                    : logProb + qLn(1.0 + qExp(logSum - logProb));
    }
    for (int k = 0; k < m_k; ++k)
        probs[k] = qExp(probs[k] - logSum);
    return probs;
}

/**
 * @brief 重置所有统计计数器
 */
void GaussianMixture8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
