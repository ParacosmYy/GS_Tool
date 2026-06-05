/**
 * @file TridiagonalEigen3.cpp
 * @brief 三对角矩阵特征值求解器实现
 *
 * 使用隐式QR算法求解对称三对角矩阵的全部特征值和特征向量。
 * 通过Wilkinson位移加速收敛，使用Givens旋转保持三对角结构。
 * 适用于由Lanczos或Householder三对角化得到的矩阵。
 */

#include "utils/matrix78/TridiagonalEigen3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象指针
 *
 * 默认容差1e-12，最大迭代1000次。
 */
TridiagonalEigen3::TridiagonalEigen3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 设置收敛容差
 * @param tol 特征值收敛容差，推荐1e-12
 */
void TridiagonalEigen3::setTolerance(double tol)
{
    m_tolerance = qBound(1e-16, tol, 1.0);
}

/**
 * @brief 设置最大迭代次数
 * @param maxIter 最大QR迭代次数
 */
void TridiagonalEigen3::setMaxIterations(int maxIter)
{
    m_maxIterations = qMax(10, maxIter);
}

/**
 * @brief 从对角线和次对角线求解特征值
 * @param diag 主对角线元素(d0, d1, ..., dn-1)
 * @param subdiag 次对角线元素(e0, e1, ..., en-2)
 * @return 特征值向量(按升序排列)
 *
 * 使用隐式QR算法:
 * 1. 复制对角线到工作数组
 * 2. 从底部开始对每个子矩阵执行QR迭代
 * 3. Wilkinson位移: d[l] + d[l+1])/2 + sign*delta/2
 * 4. Givens旋转消元并保持三对角结构
 * 5. 收敛后输出特征值
 */
QVector<double> TridiagonalEigen3::eigenvalues(const QVector<double>& diag, const QVector<double>& subdiag)
{
    QElapsedTimer timer;
    timer.start();

    int n = diag.size();
    if (n == 0) return QVector<double>();

    /* 工作数组 */
    QVector<double> d = diag;
    QVector<double> e(n, 0.0);
    for (int i = 0; i < qMin(subdiag.size(), n - 1); ++i) {
        e[i] = subdiag[i];
    }

    /* 隐式QR迭代 */
    for (int l = 0; l < n - 1; ++l) {
        int iter = 0;
        int m = l;

        while (m < n - 1) {
            /* 检查次对角线元素是否可忽略 */
            double dd = qAbs(d[m]) + qAbs(d[m + 1]);
            if (qAbs(e[m]) <= m_tolerance * dd) break;
            m++;
        }

        if (m == l) continue;

        if (iter >= m_maxIterations) break;

        /* Wilkinson位移 */
        double g = (d[l + 1] - d[l]) / (2.0 * e[l]);
        double r = qSqrt(g * g + 1.0);
        double shift = d[m] - d[l] + e[l] / (g + (g >= 0 ? qAbs(r) : -qAbs(r)));

        /* 隐式QR步: Givens旋转 */
        double c = 1.0, s = 1.0;
        double p = 0.0;

        for (int i = l; i < m; ++i) {
            double f = s * e[i];
            double b = c * e[i];

            if (qAbs(f) >= qAbs(shift)) {
                c = shift / f;
                r = qSqrt(c * c + 1.0);
                e[i] = f * r;
                s = 1.0 / r;
                c = c * s;
            } else {
                s = f / shift;
                r = qSqrt(s * s + 1.0);
                e[i] = shift * r;
                c = 1.0 / r;
                s = s * c;
            }

            shift = d[i + 1] - p;
            r = (d[i] - shift) * s + 2.0 * c * b;
            p = s * r;
            d[i] = shift + p;
            shift = c * r - b;
        }

        d[l] -= p;
        e[l] = shift;
        e[m] = 0.0;

        iter++;
        l--; /* 重新检查 */
    }

    /* 按升序排序 */
    std::sort(d.begin(), d.end());

    /* 更新统计 */
    qint64 elapsed = timer.elapsed();
    m_stats.totalEigensolves++;
    m_stats.totalEigenvalues += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEigensolves;

    emit eigensolveCompleted(n);
    return d;
}

/**
 * @brief 求解特征值和特征向量
 * @param diag 主对角线
 * @param subdiag 次对角线
 * @return QPair(特征值向量, 特征向量矩阵)
 *
 * 在计算特征值的同时累积Givens旋转变换，
 * 得到对应的特征向量矩阵。
 */
QPair<QVector<double>, QVector<QVector<double>>> TridiagonalEigen3::eigenDecomposition(
    const QVector<double>& diag, const QVector<double>& subdiag)
{
    QElapsedTimer timer;
    timer.start();

    int n = diag.size();
    if (n == 0) return {QVector<double>(), QVector<QVector<double>>()};

    /* 初始化特征向量矩阵为单位阵 */
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    QVector<double> d = diag;
    QVector<double> e(n, 0.0);
    for (int i = 0; i < qMin(subdiag.size(), n - 1); ++i) e[i] = subdiag[i];

    /* QR迭代 + 累积变换 */
    for (int l = 0; l < n - 1; ++l) {
        int iter = 0;
        int m = l;

        while (m < n - 1) {
            double dd = qAbs(d[m]) + qAbs(d[m + 1]);
            if (qAbs(e[m]) <= m_tolerance * dd) break;
            m++;
        }
        if (m == l) continue;
        if (iter >= m_maxIterations) break;

        double g = (d[l + 1] - d[l]) / (2.0 * e[l]);
        double r = qSqrt(g * g + 1.0);
        double shift = d[m] - d[l] + e[l] / (g + (g >= 0 ? qAbs(r) : -qAbs(r)));

        double c = 1.0, s = 1.0, p = 0.0;

        for (int i = l; i < m; ++i) {
            double f = s * e[i];
            double b = c * e[i];

            if (qAbs(f) >= qAbs(shift)) {
                c = shift / f;
                r = qSqrt(c * c + 1.0);
                e[i] = f * r;
                s = 1.0 / r;
                c = c * s;
            } else {
                s = f / shift;
                r = qSqrt(s * s + 1.0);
                e[i] = shift * r;
                c = 1.0 / r;
                s = s * c;
            }

            shift = d[i + 1] - p;
            r = (d[i] - shift) * s + 2.0 * c * b;
            p = s * r;
            d[i] = shift + p;
            shift = c * r - b;

            /* 累积Givens旋转到特征向量矩阵 */
            for (int k = 0; k < n; ++k) {
                double t = V[k][i];
                V[k][i] = c * t + s * V[k][i + 1];
                V[k][i + 1] = -s * t + c * V[k][i + 1];
            }
        }

        d[l] -= p;
        e[l] = shift;
        e[m] = 0.0;
        iter++;
        l--;
    }

    /* 特征值排序 */
    QVector<int> idx(n);
    for (int i = 0; i < n; ++i) idx[i] = i;
    std::sort(idx.begin(), idx.end(), [&](int a, int b) { return d[a] < d[b]; });

    QVector<double> evals(n);
    QVector<QVector<double>> evecs(n, QVector<double>(n));
    for (int i = 0; i < n; ++i) {
        evals[i] = d[idx[i]];
        for (int j = 0; j < n; ++j) {
            evecs[i][j] = V[j][idx[i]];
        }
    }

    qint64 elapsed = timer.elapsed();
    m_stats.totalEigensolves++;
    m_stats.totalEigenvalues += n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalEigensolves;

    emit eigensolveCompleted(n);
    return {evals, evecs};
}

/**
 * @brief 重置统计信息
 */
void TridiagonalEigen3::resetStatistics()
{
    m_stats = Stats();
    m_timeSum = 0.0;
}
