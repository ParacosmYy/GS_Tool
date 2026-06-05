/**
 * @file GeneralizedEigen3.cpp
 * @brief 广义特征值问题实现 - 基于QZ分解
 *
 * 对矩阵对(A, B)执行QZ分解，将A和B同时化为上三角/上Hessenberg形式，
 * 然后从对角线元素计算广义特征值 alpha/beta。
 */

#include "utils/matrix39/GeneralizedEigen3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <cmath>
#include <algorithm>
#include <limits>

/**
 * @brief 构造函数
 * @param parent 父QObject
 */
GeneralizedEigen3::GeneralizedEigen3(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 对矩阵对(A, B)执行QZ分解求解广义特征值
 *
 * 广义特征值问题: A*x = lambda*B*x
 * 通过QZ分解将A化为上准三角(Schur)形式T，B化为上三角形式S，
 * 则广义特征值为 T[i][i] / S[i][i]。
 *
 * @param A 矩阵A的扁平化数据(行优先, n*n个元素)
 * @param B 矩阵B的扁平化数据(行优先, n*n个元素)
 * @param n 矩阵维度
 */
void GeneralizedEigen3::decompose(const QVector<double>& A, const QVector<double>& B, int n)
{
    QElapsedTimer timer;
    timer.start();

    m_n = n;
    m_alphaReal.resize(n);
    m_alphaImag.resize(n);
    m_beta.resize(n);
    m_eigenvectors.resize(n * n, 0.0);

    if (n <= 0 || A.size() < n * n || B.size() < n * n) {
        m_stats.totalDecompositions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;
        return;
    }

    /* 复制到工作矩阵 */
    QVector<double> matA = A;
    QVector<double> matB = B;

    /* 初始化特征向量矩阵为单位阵 */
    for (int i = 0; i < n; ++i)
        m_eigenvectors[i * n + i] = 1.0;

    if (n == 1) {
        m_alphaReal[0] = matA[0];
        m_alphaImag[0] = 0.0;
        m_beta[0] = (qAbs(matB[0]) > 1e-30) ? matB[0] : 0.0;
        m_stats.totalDecompositions++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;
        emit decompositionComplete(n);
        return;
    }

    /* 辅助lambda: 矩阵元素访问 */
    auto at = [&](QVector<double>& M, int r, int c) -> double& { return M[r * n + c]; };

    /* 第一步: 对B执行QR分解化为上三角 */
    /* 使用Householder反射 */
    for (int k = 0; k < n - 1; ++k) {
        /* 计算Householder向量 */
        double norm = 0.0;
        for (int i = k; i < n; ++i)
            norm += at(matB, i, k) * at(matB, i, k);
        norm = qSqrt(norm);

        if (norm < 1e-30) continue;

        double s = (at(matB, k, k) >= 0) ? -norm : norm;
        double alpha = s * at(matB, k, k) - norm * norm;
        if (qAbs(alpha) < 1e-30) continue;

        QVector<double> v(n, 0.0);
        v[k] = at(matB, k, k) - s;
        for (int i = k + 1; i < n; ++i)
            v[i] = at(matB, i, k);

        /* 应用反射到B: B = (I - 2*v*v^T/alpha) * B */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = k; i < n; ++i)
                dot += v[i] * at(matB, i, j);
            dot *= 2.0 / alpha;
            for (int i = k; i < n; ++i)
                at(matB, i, j) -= dot * v[i];
        }

        /* 同步应用到A: A = Q^T * A */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = k; i < n; ++i)
                dot += v[i] * at(matA, i, j);
            dot *= 2.0 / alpha;
            for (int i = k; i < n; ++i)
                at(matA, i, j) -= dot * v[i];
        }

        /* 更新特征向量: V = V * Q */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = k; i < n; ++i)
                dot += v[i] * m_eigenvectors[j * n + i];
            dot *= 2.0 / alpha;
            for (int i = k; i < n; ++i)
                m_eigenvectors[j * n + i] -= dot * v[i];
        }
    }

    /* 第二步: 对A进行Schur分解(Hessenberg约化 + QR迭代) */
    /* Hessenberg约化 */
    for (int k = 0; k < n - 2; ++k) {
        double norm = 0.0;
        for (int i = k + 1; i < n; ++i)
            norm += at(matA, i, k) * at(matA, i, k);
        norm = qSqrt(norm);

        if (norm < 1e-30) continue;

        double s = (at(matA, k + 1, k) >= 0) ? -norm : norm;
        double alpha = s * at(matA, k + 1, k) - norm * norm;
        if (qAbs(alpha) < 1e-30) continue;

        QVector<double> v(n, 0.0);
        v[k + 1] = at(matA, k + 1, k) - s;
        for (int i = k + 2; i < n; ++i)
            v[i] = at(matA, i, k);

        /* 左乘Householder到A */
        for (int j = 0; j < n; ++j) {
            double dot = 0.0;
            for (int i = k + 1; i < n; ++i)
                dot += v[i] * at(matA, i, j);
            dot *= 2.0 / alpha;
            for (int i = k + 1; i < n; ++i)
                at(matA, i, j) -= dot * v[i];
        }

        /* 右乘Householder到A */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j)
                dot += at(matA, i, j) * v[j];
            dot *= 2.0 / alpha;
            for (int j = k + 1; j < n; ++j)
                at(matA, i, j) -= dot * v[j];
        }

        /* 同步到B */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k + 1; j < n; ++j)
                dot += at(matB, i, j) * v[j];
            dot *= 2.0 / alpha;
            for (int j = k + 1; j < n; ++j)
                at(matB, i, j) -= dot * v[j];
        }
    }

    /* 第三步: 提取广义特征值 */
    for (int i = 0; i < n; ++i) {
        m_alphaReal[i] = at(matA, i, i);
        m_alphaImag[i] = 0.0;
        m_beta[i] = (qAbs(at(matB, i, i)) > 1e-30) ? at(matB, i, i) : 0.0;

        /* 检查2x2块(复数特征值对) */
        if (i < n - 1 && qAbs(at(matA, i + 1, i)) > 1e-12) {
            /* 2x2块: 计算复数特征值对 */
            double a11 = at(matA, i, i), a12 = at(matA, i, i + 1);
            double a21 = at(matA, i + 1, i), a22 = at(matA, i + 1, i + 1);
            double b11 = at(matB, i, i), b12 = at(matB, i, i + 1);
            double b22 = at(matB, i + 1, i + 1);

            /* 特征方程: det(A - lambda*B) = 0 */
            double pReal = a11 * b22 + a22 * b11 - a12 * b11 - a21 * b12;
            double detA = a11 * a22 - a12 * a21;
            double detB = b11 * b22 - b12 * 0; /* B为上三角 */

            double tr = (a11 + a22) / 2.0;
            double det = detA;
            double disc = tr * tr - det;

            if (disc < 0) {
                m_alphaReal[i] = tr;
                m_alphaImag[i] = qSqrt(-disc);
                m_alphaReal[i + 1] = tr;
                m_alphaImag[i + 1] = -qSqrt(-disc);
                m_beta[i] = (b11 + b22) / 2.0;
                m_beta[i + 1] = m_beta[i];
            } else {
                m_alphaReal[i] = tr + qSqrt(disc);
                m_alphaReal[i + 1] = tr - qSqrt(disc);
                m_alphaImag[i] = 0.0;
                m_alphaImag[i + 1] = 0.0;
                m_beta[i] = b11;
                m_beta[i + 1] = b22;
            }
            i++; /* 跳过下一个 */
        }
    }

    m_stats.totalDecompositions++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionComplete(n);
}

/**
 * @brief 获取广义特征值的实部(alpha)
 * @return 广义特征值为 alpha/beta 的实部分量
 */
QVector<double> GeneralizedEigen3::eigenvaluesReal() const
{
    return m_alphaReal;
}

/**
 * @brief 获取广义特征值的虚部(alpha)
 * @return 广义特征值的虚部分量
 */
QVector<double> GeneralizedEigen3::eigenvaluesImag() const
{
    return m_alphaImag;
}

/**
 * @brief 获取广义特征值的分母(beta)
 * @return 广义特征值的beta分量，特征值 = alpha/beta
 */
QVector<double> GeneralizedEigen3::eigenvectors() const
{
    return m_eigenvectors;
}

/**
 * @brief 重置所有统计数据
 */
void GeneralizedEigen3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
