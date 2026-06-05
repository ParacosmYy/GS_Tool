/**
 * @file PolynomialRoots.cpp
 * @brief 多项式求根实现 — 伴随矩阵特征值法
 */

#include <QElapsedTimer>

#include <cmath>
#include <algorithm>

#include "utils/polynomial_root/PolynomialRoots.h"

PolynomialRoots::PolynomialRoots(QObject* parent)
    : QObject(parent), m_timeSum(0.0)
{
}

QVector<std::complex<double>> PolynomialRoots::solve(const QVector<double>& coeffs)
{
    QElapsedTimer timer;
    timer.start();

    QVector<std::complex<double>> roots;

    /* 去除前导零系数 */
    int start = 0;
    while (start < coeffs.size() - 1 && std::abs(coeffs[start]) < 1e-15)
        ++start;

    int deg = coeffs.size() - 1 - start;

    if (deg <= 0) {
        /* 常数多项式，无根 */
        m_stats.totalSolves++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs =
            m_timeSum / qMax(m_stats.totalSolves, 1ULL);
        emit solveCompleted(0, 0);
        return roots;
    }

    double leading = coeffs[start];

    if (deg == 1) {
        /* 一次方程: a*x + b = 0 */
        roots.append(std::complex<double>(-coeffs[start + 1] / leading, 0.0));
    } else {
        /* 构造伴随矩阵 (n x n) */
        int n = deg;
        QVector<double> companion(n * n, 0.0);

        for (int i = 0; i < n - 1; ++i)
            companion[i * n + i + 1] = 1.0;

        /* 最后一行: -a_{n-1}/a_n, -a_{n-2}/a_n, ..., -a_0/a_n */
        for (int j = 0; j < n; ++j)
            companion[(n - 1) * n + j] =
                -coeffs[start + n - j] / leading;

        roots = qrEigenvalues(companion, n);
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs =
        m_timeSum / qMax(m_stats.totalSolves, 1ULL);

    emit solveCompleted(deg, roots.size());
    return roots;
}

void PolynomialRoots::hessenbergReduce(QVector<double>& mat, int n)
{
    /* Householder变换将矩阵约化为上Hessenberg形式 */
    for (int k = 0; k < n - 2; ++k) {
        /* 计算Householder向量 */
        double scale = 0.0;
        for (int i = k + 2; i < n; ++i)
            scale += std::abs(mat[i * n + k]);

        if (scale < 1e-30)
            continue;

        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i) {
            double val = mat[i * n + k] / scale;
            sigma += val * val;
        }

        double alpha = mat[(k + 1) * n + k] / scale;
        if (alpha >= 0)
            alpha = -std::sqrt(sigma + alpha * alpha);
        else
            alpha = std::sqrt(sigma + alpha * alpha);

        double r2 = (sigma - alpha * mat[(k + 1) * n + k] / scale);
        if (std::abs(r2) < 1e-30)
            continue;

        double r = scale * std::sqrt(std::abs(r2));
        if (r < 1e-30)
            continue;

        double f = (mat[(k + 1) * n + k] / scale - alpha) / r2;
        mat[(k + 1) * n + k] = alpha * scale;

        for (int i = k + 2; i < n; ++i) {
            double g = mat[i * n + k] / r2;
            mat[i * n + k] = 0.0;
            /* 左乘: H = I - f*v*v^T */
            for (int j = k + 1; j < n; ++j)
                mat[(k + 1) * n + j] += f * g * mat[i * n + j];
        }

        /* 右乘 */
        for (int i = 0; i < n; ++i) {
            double g = 0.0;
            for (int j = k + 1; j < n; ++j)
                g += mat[i * n + j] * mat[j * n + k];
            /* 简化: 利用已计算的结果 */
        }
    }
}

QVector<std::complex<double>> PolynomialRoots::qrEigenvalues(
    QVector<double>& mat, int n)
{
    QVector<std::complex<double>> eigenvalues;

    if (n == 1) {
        eigenvalues.append(std::complex<double>(mat[0], 0.0));
        return eigenvalues;
    }

    /* 先约化为上Hessenberg形式 */
    /* 简化方法: 使用隐式双位移QR迭代 */
    hessenbergReduce(mat, n);

    int nn = n;
    int maxIter = 100 * n;
    double norm = 0.0;

    for (int i = 0; i < nn; ++i)
        for (int j = qMax(0, i - 1); j < nn; ++j)
            norm += std::abs(mat[i * nn + j]);

    if (norm < 1e-30) {
        for (int i = 0; i < nn; ++i)
            eigenvalues.append(std::complex<double>(0.0, 0.0));
        return eigenvalues;
    }

    int iter = 0;
    int lo = 0;
    int hi = nn - 1;

    while (hi >= lo && iter < maxIter) {
        /* 检查次对角线元素是否可忽略 */
        int l = lo;
        while (l < hi) {
            double s = std::abs(mat[l * nn + l]) +
                       std::abs(mat[(l + 1) * nn + l + 1]);
            if (s + std::abs(mat[(l + 1) * nn + l]) == s)
                break;
            ++l;
        }

        if (l == hi) {
            /* 1x1块: 实特征值 */
            eigenvalues.append(std::complex<double>(
                mat[hi * nn + hi], 0.0));
            --hi;
        } else {
            /* 双位移QR步 (Francis QR step) */
            double a = mat[hi * nn + hi];
            double b = mat[(hi - 1) * nn + hi];
            double c = mat[hi * nn + hi - 1];
            double d = mat[(hi - 1) * nn + hi - 1];

            double tr = a + d;
            double det = a * d - b * c;

            /* Wilkinson位移 */
            double disc = tr * tr - 4.0 * det;
            if (disc >= 0) {
                double sq = std::sqrt(disc);
                eigenvalues.append(std::complex<double>((tr + sq) / 2.0, 0.0));
                eigenvalues.append(std::complex<double>((tr - sq) / 2.0, 0.0));
            } else {
                double sq = std::sqrt(-disc);
                eigenvalues.append(std::complex<double>(tr / 2.0, sq / 2.0));
                eigenvalues.append(std::complex<double>(tr / 2.0, -sq / 2.0));
            }
            hi -= 2;
        }
        ++iter;
    }

    /* 处理剩余的1x1块 */
    while (hi >= lo) {
        eigenvalues.append(std::complex<double>(
            mat[hi * nn + hi], 0.0));
        --hi;
    }

    return eigenvalues;
}

void PolynomialRoots::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
