/**
 * @file SymmetricEigen5.cpp
 * @brief 对称矩阵特征值分解实现 — Householder三对角化 + 隐式QR迭代
 * @author EmbedDebug Team
 * @date 2026-06-05
 *
 * 实现对称矩阵特征值/特征向量计算：
 * 1. Householder 变换将对称矩阵三对角化
 * 2. 隐式 QR 迭代（带 Wilkinson 位移）求特征值
 * 3. 累积正交变换得到特征向量
 */

#include "utils/matrix38/SymmetricEigen5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <cmath>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
SymmetricEigen5::SymmetricEigen5(QObject *parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SymmetricEigen5"));
}

/**
 * @brief 重置统计信息
 */
void SymmetricEigen5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/**
 * @brief 对称矩阵特征值分解
 *
 * 输入为一维存储的对称矩阵（行优先，仅存下三角或全矩阵均可）。
 * n 阶矩阵需要 n*n 个元素。
 *
 * 处理流程：
 * 1. Householder 三对角化
 * 2. 隐式 QR 迭代求特征值
 * 3. 特征向量通过累积变换矩阵获得
 *
 * @param matrix 一维数组表示的 n x n 对称矩阵
 * @param n 矩阵阶数
 */
void SymmetricEigen5::decompose(const QVector<double> &matrix, int n)
{
    QElapsedTimer timer;
    timer.start();

    m_n = n;

    // 复制矩阵到工作数组（n x n 二维展开为一维）
    QVector<double> A = matrix;
    A.resize(n * n);

    // 初始化正交变换累积矩阵为单位矩阵
    m_eigenvectors.resize(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        m_eigenvectors[i * n + i] = 1.0;
    }

    // 三对角化后的对角线和次对角线
    QVector<double> diag(n, 0.0);
    QVector<double> subdiag(n - 1, 0.0);

    // Householder 三对角化
    for (int k = 0; k < n - 2; ++k) {
        // 计算 Householder 向量
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i) {
            sigma += A[i * n + k] * A[i * n + k];
        }

        double alpha = A[(k + 1) * n + k];
        double r = qSqrt(alpha * alpha + sigma);

        if (r < 1e-15) continue;

        double v0 = alpha;
        if (alpha >= 0) v0 += r;
        else v0 -= r;

        // 归一化 Householder 向量
        QVector<double> v(n, 0.0);
        v[k + 1] = 1.0;
        double vNorm = v0 * v0 + sigma;
        if (vNorm < 1e-30) continue;

        double scale = v0 / qSqrt(vNorm);
        v[k + 1] = scale;
        for (int i = k + 2; i < n; ++i) {
            v[i] = A[i * n + k] / qSqrt(vNorm);
        }

        // P = I - 2*v*v^T 变换: A = P*A*P
        // 先计算 p = A*v
        QVector<double> p(n, 0.0);
        for (int i = k; i < n; ++i) {
            for (int j = k; j < n; ++j) {
                p[i] += A[i * n + j] * v[j];
            }
        }

        // 计算 K = 2*p
        double vDotP = 0.0;
        for (int i = k; i < n; ++i) {
            vDotP += v[i] * p[i];
        }

        // q = p - (vDotP)*v
        QVector<double> q(n, 0.0);
        for (int i = k; i < n; ++i) {
            q[i] = p[i] - vDotP * v[i];
        }

        // 更新 A: A = A - v*q^T - q*v^T
        for (int i = k; i < n; ++i) {
            for (int j = k; j < n; ++j) {
                A[i * n + j] -= v[i] * q[j] + q[i] * v[j];
            }
        }

        // 累积变换到特征向量矩阵
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = k; j < n; ++j) {
                dot += m_eigenvectors[i * n + j] * v[j];
            }
            for (int j = k; j < n; ++j) {
                m_eigenvectors[i * n + j] -= 2.0 * dot * v[j];
            }
        }
    }

    // 提取三对角线元素
    for (int i = 0; i < n; ++i) {
        diag[i] = A[i * n + i];
    }
    for (int i = 0; i < n - 1; ++i) {
        subdiag[i] = A[(i + 1) * n + i];
    }

    // 隐式 QR 迭代（带 Wilkinson 位移）
    int lo = 0;
    int hi = n - 1;
    const int maxIter = 30 * n;

    for (int iter = 0; iter < maxIter && lo < hi; ++iter) {
        // 检查次对角线元素是否可忽略
        while (hi > lo) {
            double off = qFabs(subdiag[hi - 1]);
            double sum = qFabs(diag[hi - 1]) + qFabs(diag[hi]);
            if (off <= 1e-14 * sum) {
                subdiag[hi - 1] = 0.0;
                --hi;
            } else {
                break;
            }
        }

        if (hi <= lo) break;

        // 寻找最低未收敛的 lo
        int newLo = hi - 1;
        while (newLo > lo) {
            double off = qFabs(subdiag[newLo - 1]);
            double sum = qFabs(diag[newLo - 1]) + qFabs(diag[newLo]);
            if (off <= 1e-14 * sum) {
                subdiag[newLo - 1] = 0.0;
                break;
            }
            --newLo;
        }
        lo = newLo;

        if (hi <= lo) break;

        // Wilkinson 位移
        const double d = (diag[hi - 1] - diag[hi]) / 2.0;
        const double dd = d * d + subdiag[hi - 1] * subdiag[hi - 1];
        double mu = diag[hi];
        if (dd > 1e-30) {
            mu -= d + (d >= 0 ? 1.0 : -1.0) * qSqrt(dd);
        }

        // 隐式 QR 步骤（Givens 旋转）
        double x = diag[lo] - mu;
        double z = subdiag[lo];

        for (int k = lo; k < hi; ++k) {
            // 计算 Givens 旋转
            double c, s;
            if (qFabs(z) < 1e-30 && qFabs(x) < 1e-30) {
                c = 1.0; s = 0.0;
            } else {
                double r = qSqrt(x * x + z * z);
                c = x / r;
                s = -z / r;
            }

            // 应用 Givens 旋转到三对角矩阵
            if (k > lo) {
                subdiag[k - 1] = qSqrt(x * x + z * z);
            }

            const double dk = diag[k];
            const double dk1 = diag[k + 1];
            const double ek = subdiag[k];

            diag[k] = c * c * dk - 2.0 * c * s * ek + s * s * dk1;
            diag[k + 1] = s * s * dk + 2.0 * c * s * ek + c * c * dk1;
            subdiag[k] = c * s * (dk - dk1) + (c * c - s * s) * ek;

            if (k + 1 < hi) {
                x = subdiag[k] * (-s);
                z = subdiag[k + 1] * c;
                subdiag[k] = subdiag[k] * c;
            }

            // 累积 Givens 旋转到特征向量矩阵
            for (int i = 0; i < n; ++i) {
                const double v1 = m_eigenvectors[i * n + k];
                const double v2 = m_eigenvectors[i * n + k + 1];
                m_eigenvectors[i * n + k] = c * v1 - s * v2;
                m_eigenvectors[i * n + k + 1] = s * v1 + c * v2;
            }
        }

        lo = 0; // 重置 lo
    }

    m_eigenvalues = diag;

    // 按特征值降序排列
    QVector<int> indices(n);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(), [&diag](int a, int b) {
        return diag[a] > diag[b];
    });

    QVector<double> sortedEigen(n);
    QVector<double> sortedEigenVec(n * n);
    for (int i = 0; i < n; ++i) {
        sortedEigen[i] = m_eigenvalues[indices[i]];
        for (int j = 0; j < n; ++j) {
            sortedEigenVec[j * n + i] = m_eigenvectors[j * n + indices[i]];
        }
    }
    m_eigenvalues = sortedEigen;
    m_eigenvectors = sortedEigenVec;

    // 更新统计信息
    m_stats.totalDecompositions++;
    m_stats.totalEigenvalues += n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionComplete(n, n);
}

/**
 * @brief 获取特征值
 * @return 降序排列的特征值向量
 */
QVector<double> SymmetricEigen5::eigenvalues() const
{
    return m_eigenvalues;
}

/**
 * @brief 获取特征向量
 *
 * 特征向量按列存储，第 i 列对应 eigenvalues()[i]。
 * 存储为一维数组：eigenvectors()[row * n + col]。
 *
 * @return 特征向量矩阵（一维存储，列优先）
 */
QVector<double> SymmetricEigen5::eigenvectors() const
{
    return m_eigenvectors;
}

/**
 * @brief 计算矩阵的秩
 *
 * 秩等于非零特征值的个数。
 *
 * @param tol 判断特征值为零的阈值
 * @return 矩阵的秩
 */
int SymmetricEigen5::rank(double tol) const
{
    int r = 0;
    for (double val : m_eigenvalues) {
        if (qFabs(val) > tol) ++r;
    }
    return r;
}

/**
 * @brief 计算矩阵的条件数
 *
 * 条件数 = 最大特征值绝对值 / 最小非零特征值绝对值。
 * 矩阵奇异时返回无穷大。
 *
 * @return 条件数，奇异时返回 std::numeric_limits<double>::max()
 */
double SymmetricEigen5::conditionNumber() const
{
    if (m_eigenvalues.isEmpty()) return 0.0;

    double maxAbs = 0.0;
    double minAbs = std::numeric_limits<double>::max();

    for (double val : m_eigenvalues) {
        const double absVal = qFabs(val);
        if (absVal > maxAbs) maxAbs = absVal;
        if (absVal > 1e-15 && absVal < minAbs) minAbs = absVal;
    }

    if (minAbs < 1e-15) return std::numeric_limits<double>::max();
    return maxAbs / minAbs;
}
