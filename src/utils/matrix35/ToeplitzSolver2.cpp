/**
 * @file ToeplitzSolver2.cpp
 * @brief Toeplitz求解器增强实现 — Levinson递归/Trench逆/Yule-Walker
 */

#include "utils/matrix35/ToeplitzSolver2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
ToeplitzSolver2::ToeplitzSolver2(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief Levinson-Durbin递归求解Toeplitz系统 Tx = b
 * @param col Toeplitz矩阵第一列(t_0, t_1, ..., t_{n-1})
 * @param rhs 右端向量
 * @return 解向量x
 */
QVector<double> ToeplitzSolver2::solve(const QVector<double>& col,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = col.size();
    if (n == 0 || rhs.size() != n) return {};

    m_n = n;

    /* Levinson递归:
     * 维护前向/后向反射系数和辅助向量
     * T_m * f_m = alpha_m, T_m * b_m = beta_m
     * 其中alpha和beta由递推关系给出 */

    double t0 = col[0];
    if (qFabs(t0) < 1e-15) return QVector<double>(n, 0.0);

    /* m=1的初始解 */
    QVector<double> x(n, 0.0);
    QVector<double> f(n, 0.0); /* 前向辅助向量 */
    QVector<double> b(n, 0.0); /* 后向辅助向量 */

    x[0] = rhs[0] / t0;
    f[0] = 1.0 / t0;
    b[0] = 1.0 / t0;

    double alpha = t0; /* 当前主子式的行列式 */

    for (int m = 1; m < n; ++m) {
        /* 计算误差项 */
        double ef = 0.0;
        double eb = 0.0;
        double ex = 0.0;

        for (int i = 0; i < m; ++i) {
            ef += col[m - i] * f[i];
            eb += col[i + 1] * b[i];
            ex += col[m - i] * x[i];
        }

        double denom = 1.0 - ef * eb;
        if (qFabs(denom) < 1e-15) break;

        /* 更新反射系数 */
        double km = ef / denom;
        double km2 = eb / denom;

        /* 更新前向和后向向量 */
        QVector<double> fNew = f;
        QVector<double> bNew = b;

        for (int i = 0; i < m; ++i) {
            fNew[i] = f[i] - km * b[i];
            bNew[i + 1] = b[i] - km2 * f[i];
        }
        fNew[m] = -km;
        bNew[0] = -km2;

        f = fNew;
        b = bNew;

        /* 更新解向量 */
        double exErr = rhs[m] - ex;
        for (int i = 0; i <= m; ++i) {
            x[i] += exErr * b[i];
        }

        /* 归一化 */
        alpha *= denom;
    }

    m_stats.totalSolves++;
    m_stats.totalSystemsSize += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSolves));

    emit solveComplete(n);
    return x;
}

/**
 * @brief Trench算法求Toeplitz矩阵的逆
 * @param col Toeplitz矩阵第一列
 * @return 逆矩阵(展平为一维, 行优先)
 */
QVector<double> ToeplitzSolver2::inverse(const QVector<double>& col)
{
    QElapsedTimer timer;
    timer.start();

    int n = col.size();
    if (n == 0) return {};

    m_n = n;
    double t0 = col[0];
    if (qFabs(t0) < 1e-15) return QVector<double>(n * n, 0.0);

    /* 使用Levinson递归构建逆矩阵 */
    /* 先求解单位矩阵的每一列 */
    QVector<double> inv(n * n, 0.0);

    /* Trench算法: O(n^2)复杂度 */
    QVector<double> v(n, 0.0);
    v[0] = 1.0 / t0;

    double alpha = t0;

    for (int m = 1; m < n; ++m) {
        /* 计算误差 */
        double ef = 0.0;
        for (int i = 0; i < m; ++i) {
            ef += col[m - i] * v[i];
        }

        double denom = 1.0 - ef * ef;
        if (qFabs(denom) < 1e-15) break;

        /* 更新v */
        QVector<double> vNew(n, 0.0);
        vNew[m] = -ef / (t0 * denom);
        for (int i = 0; i < m; ++i) {
            vNew[i] = (v[i] - ef * v[m - 1 - i]) / denom;
        }
        v = vNew;
        alpha *= denom;
    }

    /* 从v重建逆矩阵: T^{-1}[i][j] = v[|i-j|] */
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            int idx = qAbs(i - j);
            inv[i * n + j] = (idx < n) ? v[idx] : 0.0;
        }
    }

    /* 对角线需要额外校正 */
    double sumCorr = 0.0;
    for (int i = 0; i < n; ++i) {
        sumCorr = 0.0;
        for (int k = 0; k < n; ++k) {
            int d = qAbs(i - k);
            if (d < n) sumCorr += col[d] * v[qAbs(i - k)];
        }
        inv[i * n + i] = (1.0 - sumCorr + col[0] * v[0]) / col[0];
    }

    m_stats.totalSolves++;
    m_stats.totalSystemsSize += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSolves));

    return inv;
}

/**
 * @brief 计算Toeplitz矩阵行列式
 * @param col Toeplitz矩阵第一列
 * @return 行列式值
 */
double ToeplitzSolver2::determinant(const QVector<double>& col)
{
    int n = col.size();
    if (n == 0) return 0.0;

    m_n = n;
    double t0 = col[0];
    if (qFabs(t0) < 1e-15) return 0.0;

    /* Levinson递归中alpha即为各阶主子式行列式 */
    double det = t0;
    double alpha = t0;

    QVector<double> v(n, 0.0);
    v[0] = 1.0 / t0;

    for (int m = 1; m < n; ++m) {
        double ef = 0.0;
        for (int i = 0; i < m; ++i) {
            ef += col[m - i] * v[i];
        }

        double denom = 1.0 - ef * ef;
        if (qFabs(denom) < 1e-15) return 0.0;

        QVector<double> vNew(n, 0.0);
        vNew[m] = -ef / (t0 * denom);
        for (int i = 0; i < m; ++i) {
            vNew[i] = (v[i] - ef * v[m - 1 - i]) / denom;
        }
        v = vNew;

        alpha *= denom;
        det = alpha;
    }

    return det;
}

/**
 * @brief Yule-Walker方程求解(用于AR模型)
 * @param autocorr 自相关序列(r_0, r_1, ..., r_{order})
 * @param order AR阶数
 * @return AR系数(a_1, a_2, ..., a_{order})
 */
QVector<double> ToeplitzSolver2::yuleWalker(const QVector<double>& autocorr, int order)
{
    QElapsedTimer timer;
    timer.start();

    if (autocorr.size() < order + 1 || order <= 0) return {};

    m_n = order;

    /* Yule-Walker: R * a = -r
     * R是order x order的Toeplitz自相关矩阵
     * r是(r_1, ..., r_order) */

    double r0 = autocorr[0];
    if (qFabs(r0) < 1e-15) return QVector<double>(order, 0.0);

    /* 构造Toeplitz列 */
    QVector<double> col(order);
    for (int i = 0; i < order; ++i) {
        col[i] = autocorr[i] / r0;
    }

    /* 构造右端向量 */
    QVector<double> rhs(order);
    for (int i = 0; i < order; ++i) {
        rhs[i] = -autocorr[i + 1] / r0;
    }

    /* Levinson-Durbin递推 */
    QVector<double> a(order, 0.0);
    QVector<double> aPrev(order, 0.0);
    double e = col[0]; /* 预测误差 */

    a[0] = rhs[0] / col[0];

    for (int m = 1; m < order; ++m) {
        /* 计算反射系数 */
        double km = rhs[m];
        for (int i = 0; i < m; ++i) {
            km += col[m - i] * aPrev[i];
        }
        km /= e;

        /* 更新AR系数 */
        a[m] = km;
        for (int i = 0; i < m; ++i) {
            a[i] = aPrev[i] - km * aPrev[m - 1 - i];
        }

        /* 更新误差 */
        e *= (1.0 - km * km);
        aPrev = a;
    }

    m_stats.totalSolves++;
    m_stats.totalSystemsSize += order;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(qMax(1, m_stats.totalSolves));

    emit solveComplete(order);
    return a;
}

/** @brief 重置统计 */
void ToeplitzSolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
