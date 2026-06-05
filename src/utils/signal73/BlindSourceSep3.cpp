/**
 * @file BlindSourceSep3.cpp
 * @brief 盲源分离(ICA)实现
 *
 * 实现基于FastICA算法的独立成分分析(ICA)盲源分离，
 * 通过最大化非高斯性从混合信号中恢复原始独立源。
 */

#include "utils/signal73/BlindSourceSep3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <random>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 */
BlindSourceSep3::BlindSourceSep3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置源信号数量
 * @param n 源信号数
 */
void BlindSourceSep3::setNumSources(int n)
{
    m_nSources = qMax(1, n);
}

/**
 * @brief 设置学习率
 * @param lr 学习率
 */
void BlindSourceSep3::setLearningRate(double lr)
{
    m_lr = qBound(1e-6, lr, 1.0);
}

/**
 * @brief 设置最大迭代次数
 * @param iter 最大迭代次数
 */
void BlindSourceSep3::setMaxIterations(int iter)
{
    m_maxIter = qMax(10, iter);
}

/**
 * @brief 设置收敛容差
 * @param tol 收敛阈值
 */
void BlindSourceSep3::setConvergenceTol(double tol)
{
    m_tol = qBound(1e-12, tol, 1e-2);
}

/**
 * @brief 对混合信号进行盲源分离
 * @param mixtures 混合信号矩阵，每行一个传感器，每列一个时间采样
 * @return 分离后的源信号矩阵
 */
QVector<QVector<double>> BlindSourceSep3::separate(const QVector<QVector<double>>& mixtures)
{
    QElapsedTimer timer;
    timer.start();

    int M = mixtures.size(); // 传感器数
    if (M == 0) return QVector<QVector<double>>();

    int T = mixtures[0].size(); // 时间采样数
    int N = qMin(M, m_nSources); // 源数不超过传感器数

    // 步骤1：中心化（减去均值）
    QVector<QVector<double>> X(M, QVector<double>(T, 0.0));
    for (int i = 0; i < M; ++i) {
        double mean = 0.0;
        for (int t = 0; t < T; ++t) mean += mixtures[i][t];
        mean /= T;
        for (int t = 0; t < T; ++t) X[i][t] = mixtures[i][t] - mean;
    }

    // 步骤2：白化（PCA + 方差归一化）
    // 计算协方差矩阵
    QVector<QVector<double>> cov(M, QVector<double>(M, 0.0));
    for (int i = 0; i < M; ++i) {
        for (int j = i; j < M; ++j) {
            double sum = 0.0;
            for (int t = 0; t < T; ++t) sum += X[i][t] * X[j][t];
            cov[i][j] = cov[j][i] = sum / T;
        }
    }

    // 简化白化：使用对角近似
    QVector<double> invStd(M, 1.0);
    for (int i = 0; i < M; ++i) {
        invStd[i] = 1.0 / qSqrt(qMax(cov[i][i], 1e-10));
    }

    // 白化后的数据
    for (int i = 0; i < M; ++i) {
        for (int t = 0; t < T; ++t) {
            X[i][t] *= invStd[i];
        }
    }

    // 步骤3：FastICA迭代
    // 初始化解混矩阵W
    std::mt19937 rng(42);
    std::normal_distribution<double> normDist(0.0, 1.0);
    m_W.resize(N, QVector<double>(N, 0.0));
    for (int i = 0; i < N; ++i) {
        for (int j = 0; j < N; ++j) {
            m_W[i][j] = normDist(rng) * 0.1;
        }
    }

    m_convergence = 1e18;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        double maxChange = 0.0;

        for (int i = 0; i < N; ++i) {
            // 计算w^T * X
            QVector<double> wX(T, 0.0);
            for (int t = 0; t < T; ++t) {
                for (int j = 0; j < N && j < M; ++j) {
                    wX[t] += m_W[i][j] * X[j][t];
                }
            }

            // FastICA更新：E{x*g(w^T*x)} - E{g'(w^T*x)}*w
            QVector<double> newW(N, 0.0);
            for (int j = 0; j < N && j < M; ++j) {
                double sum1 = 0.0, sum2 = 0.0;
                for (int t = 0; t < T; ++t) {
                    sum1 += X[j][t] * g(wX[t]);
                    sum2 += gPrime(wX[t]);
                }
                newW[j] = sum1 / T - (sum2 / T) * m_W[i][j];
            }

            // 归一化
            double norm = 0.0;
            for (double v : newW) norm += v * v;
            norm = qSqrt(norm);
            if (norm > 1e-15) {
                for (double& v : newW) v /= norm;
            }

            // 检查收敛
            double dot = 0.0;
            for (int j = 0; j < N; ++j) dot += qAbs(newW[j] * m_W[i][j]);
            double change = qAbs(1.0 - dot);
            maxChange = qMax(maxChange, change);

            m_W[i] = newW;
        }

        // 对称正交化（Gram-Schmidt）
        for (int i = 0; i < N; ++i) {
            for (int j = 0; j < i; ++j) {
                double dot = 0.0;
                for (int k = 0; k < N; ++k) dot += m_W[i][k] * m_W[j][k];
                for (int k = 0; k < N; ++k) m_W[i][k] -= dot * m_W[j][k];
            }
            double norm = 0.0;
            for (double v : m_W[i]) norm += v * v;
            norm = qSqrt(norm);
            if (norm > 1e-15) {
                for (double& v : m_W[i]) v /= norm;
            }
        }

        m_convergence = maxChange;
        if (maxChange < m_tol) break;
    }

    // 步骤4：应用解混矩阵得到源信号
    QVector<QVector<double>> sources(N, QVector<double>(T, 0.0));
    for (int i = 0; i < N; ++i) {
        for (int t = 0; t < T; ++t) {
            for (int j = 0; j < N && j < M; ++j) {
                sources[i][t] += m_W[i][j] * X[j][t];
            }
        }
    }

    // 更新统计信息
    qint64 elapsed = timer.elapsed();
    m_stats.totalSeparations++;
    m_stats.totalSamples += T;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSeparations;

    emit separationCompleted(N, m_convergence);
    return sources;
}

/**
 * @brief 重置统计信息
 */
void BlindSourceSep3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}

/**
 * @brief 非线性函数g（用于FastICA对比函数）
 * @param x 输入值
 * @return g(x) = tanh(x)
 */
double BlindSourceSep3::g(double x) const
{
    return qTanh(qBound(-10.0, x, 10.0));
}

/**
 * @brief 非线性函数g的导数
 * @param x 输入值
 * @return g'(x) = 1 - tanh^2(x)
 */
double BlindSourceSep3::gPrime(double x) const
{
    double t = qTanh(qBound(-10.0, x, 10.0));
    return 1.0 - t * t;
}
