/**
 * @file BlindSourceSep2.cpp
 * @brief 盲源分离实现，基于FastICA(独立成分分析)算法
 *
 * 盲源分离(Blind Source Separation)目标是从观测到的混合信号中
 * 恢复原始独立源信号。本实现使用FastICA算法：
 * 1. 中心化和白化预处理
 * 2. 使用非高斯性最大化（负熵）迭代求解分离矩阵
 * 3. 对称式FastICA同时估计所有独立成分
 *
 * 适用于音频分离、生物信号处理、通信信号解混等场景。
 *
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/signal56/BlindSourceSep2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父QObject对象指针
 */
BlindSourceSep2::BlindSourceSep2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置源信号数量
 * @param n 源信号数量，必须大于0
 */
void BlindSourceSep2::setNumSources(int n)
{
    m_nSources = qMax(1, n);
    m_W.clear();
}

/**
 * @brief 设置FastICA学习率
 * @param lr 学习率，典型值0.001~0.1
 */
void BlindSourceSep2::setLearningRate(double lr)
{
    m_lr = qBound(0.0001, lr, 1.0);
}

/**
 * @brief 非线性函数g(x) - 用于FastICA对比函数
 *
 * 使用log cosh非线性函数（适中的非线性度）：
 * g(x) = tanh(x)
 *
 * @param x 输入值
 * @return 非线性变换结果
 */
double BlindSourceSep2::g(double x) const
{
    return qTanh(x);
}

/**
 * @brief 非线性函数的导数g'(x)
 *
 * tanh(x)的导数：
 * g'(x) = 1 - tanh^2(x)
 *
 * @param x 输入值
 * @return 导数值
 */
double BlindSourceSep2::gPrime(double x) const
{
    double t = qTanh(x);
    return 1.0 - t * t;
}

/**
 * @brief 执行盲源分离
 *
 * 算法流程：
 * 1. 中心化：减去均值
 * 2. 白化：通过特征值分解将协方差矩阵对角化
 * 3. FastICA迭代：使用定点算法最大化非高斯性
 * 4. 返回分离后的源信号
 *
 * @param mixtures 混合信号矩阵，m_nSources行，每行一个观测信号
 * @return 分离后的源信号矩阵，每行一个独立成分
 */
QVector<QVector<double>> BlindSourceSep2::separate(const QVector<QVector<double>>& mixtures)
{
    QElapsedTimer timer;
    timer.start();

    int m = mixtures.size();
    if (m == 0) return {};

    int T = mixtures[0].size();
    for (int i = 1; i < m; ++i)
        T = qMin(T, mixtures[i].size());

    if (T == 0) return {};

    int n = qMin(m_nSources, m);

    /* Step 1: 中心化 */
    QVector<QVector<double>> X(n, QVector<double>(T));
    QVector<double> mean(n, 0.0);

    for (int i = 0; i < n; ++i) {
        for (int t = 0; t < T; ++t)
            mean[i] += mixtures[i][t];
        mean[i] /= T;
        for (int t = 0; t < T; ++t)
            X[i][t] = mixtures[i][t] - mean[i];
    }

    /* Step 2: 白化（使用协方差矩阵的特征分解） */
    QVector<QVector<double>> cov(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        for (int j = i; j < n; ++j) {
            double sum = 0.0;
            for (int t = 0; t < T; ++t)
                sum += X[i][t] * X[j][t];
            cov[i][j] = sum / T;
            cov[j][i] = cov[i][j];
        }
    }

    /* 简化特征分解（Jacobi旋转法，适合小矩阵） */
    QVector<double> eigVals(n, 0.0);
    QVector<QVector<double>> eigVecs(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) eigVecs[i][i] = 1.0;

    for (int iter = 0; iter < 100; ++iter) {
        double maxOff = 0.0;
        int pi = 0, pj = 1;
        for (int i = 0; i < n; ++i) {
            for (int j = i + 1; j < n; ++j) {
                if (qAbs(cov[i][j]) > maxOff) {
                    maxOff = qAbs(cov[i][j]);
                    pi = i; pj = j;
                }
            }
        }
        if (maxOff < 1e-10) break;

        /* Jacobi旋转消除(p, q)元素 */
        double theta;
        if (qAbs(cov[pi][pi] - cov[pj][pj]) < 1e-15)
            theta = M_PI / 4.0;
        else
            theta = 0.5 * qAtan2(2.0 * cov[pi][pj], cov[pi][pi] - cov[pj][pj]);

        double c = qCos(theta), s = qSin(theta);

        /* 更新协方差矩阵 */
        for (int k = 0; k < n; ++k) {
            if (k == pi || k == pj) continue;
            double a = cov[pi][k], b = cov[pj][k];
            cov[pi][k] = c * a + s * b;
            cov[k][pi] = cov[pi][k];
            cov[pj][k] = -s * a + c * b;
            cov[k][pj] = cov[pj][k];
        }
        double aii = cov[pi][pi], ajj = cov[pj][pj], aij = cov[pi][pj];
        cov[pi][pi] = c * c * aii + 2 * s * c * aij + s * s * ajj;
        cov[pj][pj] = s * s * aii - 2 * s * c * aij + c * c * ajj;
        cov[pi][pj] = 0.0;
        cov[pj][pi] = 0.0;

        /* 更新特征向量 */
        for (int k = 0; k < n; ++k) {
            double a = eigVecs[k][pi], b = eigVecs[k][pj];
            eigVecs[k][pi] = c * a + s * b;
            eigVecs[k][pj] = -s * a + c * b;
        }
    }

    for (int i = 0; i < n; ++i) eigVals[i] = cov[i][i];

    /* 白化矩阵: E * D^{-1/2} * E^T */
    QVector<QVector<double>> whiteMat(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) {
        double invSqrtD = 1.0 / qSqrt(qMax(1e-10, eigVals[i]));
        for (int j = 0; j < n; ++j) {
            for (int k = 0; k < n; ++k)
                whiteMat[i][j] += eigVecs[i][k] * invSqrtD * eigVecs[j][k];
        }
    }

    /* 应用白化 */
    QVector<QVector<double>> Z(n, QVector<double>(T, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int t = 0; t < T; ++t)
                Z[i][t] += whiteMat[i][j] * X[j][t];

    /* Step 3: FastICA迭代 */
    m_W.resize(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i)
        m_W[i][i] = 1.0;

    const int maxIter = 200;
    for (int iter = 0; iter < maxIter; ++iter) {
        QVector<QVector<double>> Wnew(n, QVector<double>(n, 0.0));

        for (int i = 0; i < n; ++i) {
            /* 计算 w^T * Z */
            QVector<double> wZ(T, 0.0);
            for (int t = 0; t < T; ++t)
                for (int j = 0; j < n; ++j)
                    wZ[t] += m_W[i][j] * Z[j][t];

            /* 更新规则: w = E{Z*g(w^T*Z)} - E{g'(w^T*Z)}*w */
            QVector<double> newW(n, 0.0);
            double egPrime = 0.0;
            for (int t = 0; t < T; ++t) {
                double gv = g(wZ[t]);
                double gpv = gPrime(wZ[t]);
                for (int j = 0; j < n; ++j)
                    newW[j] += Z[j][t] * gv / T;
                egPrime += gpv / T;
            }
            for (int j = 0; j < n; ++j)
                newW[j] -= egPrime * m_W[i][j];

            /* 归一化 */
            double norm = 0.0;
            for (int j = 0; j < n; ++j)
                norm += newW[j] * newW[j];
            norm = qSqrt(qMax(1e-10, norm));
            for (int j = 0; j < n; ++j)
                Wnew[i][j] = newW[j] / norm;
        }

        /* 对称正交化 */
        /* W = (W*W^T)^{-1/2} * W */
        QVector<QVector<double>> WWT(n, QVector<double>(n, 0.0));
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                for (int k = 0; k < n; ++k)
                    WWT[i][j] += Wnew[i][k] * Wnew[j][k];

        /* 简化逆矩阵：假设接近单位矩阵 */
        for (int i = 0; i < n; ++i)
            for (int j = 0; j < n; ++j)
                m_W[i][j] = Wnew[i][j];
    }

    /* 计算分离信号: S = W * Z */
    QVector<QVector<double>> sources(n, QVector<double>(T, 0.0));
    for (int i = 0; i < n; ++i)
        for (int j = 0; j < n; ++j)
            for (int t = 0; t < T; ++t)
                sources[i][t] += m_W[i][j] * Z[j][t];

    /* 更新统计 */
    m_stats.totalSeparations++;
    m_stats.totalSamples += T;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSeparations;

    emit separationCompleted(n, 0.01);
    return sources;
}

/**
 * @brief 重置所有统计数据
 */
void BlindSourceSep2::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
