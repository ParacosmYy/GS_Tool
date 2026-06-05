/**
 * @file SchurDecomp6.cpp
 * @brief Schur分解求解器实现 — QR迭代法求实Schur形式
 */

#include "utils/matrix82/SchurDecomp6.h"

#include <QElapsedTimer>

#include <cmath>
#include <complex>

/** @brief 构造函数 @param parent 父对象 */
SchurDecomp6::SchurDecomp6(QObject* parent)
    : QObject(parent)
    , m_size(0)
{
}

/**
 * @brief 执行实Schur分解 A = Q*T*Q^T
 * @param matrix 输入方阵(n x n)
 * @return 分解是否收敛
 */
bool SchurDecomp6::decompose(const QVector<QVector<double>>& matrix)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return false;

    /* 验证方阵 */
    for (int i = 0; i < n; ++i) {
        if (matrix[i].size() != n) return false;
    }

    m_size = n;

    /* 初始化: T = A, Q = I */
    m_T = matrix;
    m_Q.clear();
    m_Q.resize(n);
    for (int i = 0; i < n; ++i) {
        m_Q[i].resize(n, 0.0);
        m_Q[i][i] = 1.0;
    }

    /* --- Step 1: Hessenberg化简 A = Q0 * H * Q0^T ---
     * 将矩阵变换为上Hessenberg形式, 减少后续QR迭代的计算量 */
    for (int k = 0; k < n - 2; ++k) {
        /* 构造Householder向量: 消去第k列主对角线下方第2行起的元素 */
        int m = n - k - 1; /* 子向量长度 */
        QVector<double> v(m);
        double beta = 0.0;

        /* 提取列k中从k+1到n-1的元素 */
        double sigma = 0.0;
        for (int i = k + 2; i < n; ++i) {
            sigma += m_T[i][k] * m_T[i][k];
        }

        if (sigma < 1e-30 && std::abs(m_T[k + 1][k]) < 1e-30) continue;

        double alpha = m_T[k + 1][k];
        double normX = std::sqrt(alpha * alpha + sigma);

        if (std::abs(normX) < 1e-30) continue;

        v[0] = (alpha >= 0) ? alpha + normX : alpha - normX;
        for (int i = 1; i < m; ++i) {
            v[i] = m_T[k + 1 + i][k];
        }

        double v0sq = v[0] * v[0];
        beta = -2.0 * v0sq / (v0sq + sigma);

        /* 归一化v */
        double vNorm = std::sqrt(v0sq + sigma);
        if (std::abs(vNorm) < 1e-30) continue;
        for (int i = 0; i < m; ++i) v[i] /= vNorm;
        beta = -2.0;

        /* 左乘: T(k+1:n, k:n) = (I + beta*v*v^T) * T(k+1:n, k:n) */
        for (int j = k; j < n; ++j) {
            double dot = 0.0;
            for (int i = 0; i < m; ++i) {
                dot += v[i] * m_T[k + 1 + i][j];
            }
            for (int i = 0; i < m; ++i) {
                m_T[k + 1 + i][j] += beta * v[i] * dot;
            }
        }

        /* 右乘: T(0:n, k+1:n) = T(0:n, k+1:n) * (I + beta*v*v^T) */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < m; ++j) {
                dot += m_T[i][k + 1 + j] * v[j];
            }
            for (int j = 0; j < m; ++j) {
                m_T[i][k + 1 + j] += beta * dot * v[j];
            }
        }

        /* 累积正交变换到Q */
        for (int i = 0; i < n; ++i) {
            double dot = 0.0;
            for (int j = 0; j < m; ++j) {
                dot += m_Q[i][k + 1 + j] * v[j];
            }
            for (int j = 0; j < m; ++j) {
                m_Q[i][k + 1 + j] += beta * dot * v[j];
            }
        }
    }

    /* --- Step 2: 隐式双移QR迭代 --- */
    const int maxIter = 30 * n;
    int p = n - 1; /* 当前收缩位置 */
    int iterCount = 0;
    bool converged = true;

    while (p > 0 && iterCount < maxIter) {
        /* 检查次对角线元素T(p, p-1)是否可忽略 */
        double sp = std::abs(m_T[p][p - 1]);
        double ap = std::abs(m_T[p - 1][p - 1]) + std::abs(m_T[p][p]);

        if (sp <= 1e-14 * ap || sp < 1e-30) {
            /* 1x1块: 收缩 */
            m_T[p][p - 1] = 0.0;
            p--;
            iterCount = 0;
            continue;
        }

        if (p > 1) {
            double sp1 = std::abs(m_T[p - 1][p - 2]);
            double ap1 = std::abs(m_T[p - 2][p - 2]) + std::abs(m_T[p - 1][p - 1]);
            if (sp1 <= 1e-14 * ap1 || sp1 < 1e-30) {
                m_T[p - 1][p - 2] = 0.0;
            }
        }

        /* Wilkinson位移: T(p-1:p, p-1:p)的特征值 */
        double a = m_T[p - 1][p - 1];
        double b = m_T[p - 1][p];
        double c = m_T[p][p - 1];
        double d = m_T[p][p];
        double trace = a + d;
        double det = a * d - b * c;
        double disc = trace * trace - 4.0 * det;
        double sqrtDisc = std::sqrt(std::abs(disc));

        /* 双移参数 */
        double s = trace;
        double t = det;

        /* 隐式QR步: 构造第一列位移多项式 */
        double x = m_T[0][0] * m_T[0][0] + m_T[0][1] * m_T[1][0] - s * m_T[0][0] + t;
        double y = m_T[1][0] * (m_T[0][0] + m_T[1][1] - s);
        double z = m_T[1][0] * m_T[2][1];

        /* 追赶法(Bulge Chase) */
        for (int k = 0; k <= p - 2; ++k) {
            /* 构造3x1向量 [x, y, z]^T 的Householder反射 */
            double norm = std::sqrt(x * x + y * y + z * z);
            if (norm < 1e-30) break;

            QVector<double> u = {x, y, z};
            if (u[0] >= 0) norm = -norm;
            u[0] -= norm;
            double uNorm = std::sqrt(u[0] * u[0] + u[1] * u[1] + u[2] * u[2]);
            if (uNorm < 1e-30) break;
            u[0] /= uNorm; u[1] /= uNorm; u[2] /= uNorm;

            int r = std::min(k + 4, p + 1); /* 受影响行数上限 */

            /* 右乘: T(k:k+2, :) */
            for (int j = 0; j < r; ++j) {
                double dot = u[0] * m_T[k][j] + u[1] * m_T[k + 1][j] + u[2] * m_T[k + 2][j];
                m_T[k][j] -= 2.0 * u[0] * dot;
                m_T[k + 1][j] -= 2.0 * u[1] * dot;
                m_T[k + 2][j] -= 2.0 * u[2] * dot;
            }

            /* 左乘: T(:, k:k+2) */
            for (int j = 0; j < r; ++j) {
                double dot = u[0] * m_T[j][k] + u[1] * m_T[j][k + 1] + u[2] * m_T[j][k + 2];
                m_T[j][k] -= 2.0 * u[0] * dot;
                m_T[j][k + 1] -= 2.0 * u[1] * dot;
                m_T[j][k + 2] -= 2.0 * u[2] * dot;
            }

            /* 累积到Q */
            for (int j = 0; j < n; ++j) {
                double dot = u[0] * m_Q[j][k] + u[1] * m_Q[j][k + 1] + u[2] * m_Q[j][k + 2];
                m_Q[j][k] -= 2.0 * u[0] * dot;
                m_Q[j][k + 1] -= 2.0 * u[1] * dot;
                m_Q[j][k + 2] -= 2.0 * u[2] * dot;
            }

            /* 准备下一次追赶 */
            if (k < p - 2) {
                x = m_T[k + 1][k];
                y = m_T[k + 2][k];
                z = (k + 3 <= p) ? m_T[k + 3][k] : 0.0;
            }
        }

        /* 最后一个2x2 Givens旋转 */
        if (p >= 1) {
            double a2 = m_T[p - 1][p - 1];
            double b2 = m_T[p - 1][p];
            double c2 = m_T[p][p - 1];
            double d2 = m_T[p][p];

            /* 如果2x2块已收敛则收缩 */
            if (std::abs(c2) <= 1e-14 * (std::abs(a2) + std::abs(d2))) {
                m_T[p][p - 1] = 0.0;
                p -= 2;
                iterCount = 0;
            }
        }

        iterCount++;
    }

    if (iterCount >= maxIter) converged = false;

    /* --- 更新统计 --- */
    m_stats.totalDecompositions++;
    m_stats.totalEigenvalues += n;
    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(n, converged);
    return converged;
}

/**
 * @brief 获取Schur形式的上三角矩阵T
 * @return T矩阵(Q^T * A * Q的结果)
 */
QVector<QVector<double>> SchurDecomp6::schurForm() const
{
    return m_T;
}

/**
 * @brief 获取正交矩阵Q
 * @return Q矩阵, 满足 A = Q*T*Q^T
 */
QVector<QVector<double>> SchurDecomp6::unitaryMatrix() const
{
    return m_Q;
}

/**
 * @brief 从Schur形式提取特征值(含复数对)
 * @return 特征值数组(可能含复数)
 */
QVector<std::complex<double>> SchurDecomp6::eigenvalues() const
{
    QVector<std::complex<double>> eigs;
    int n = m_size;
    int i = 0;

    while (i < n) {
        if (i == n - 1 || std::abs(m_T[i + 1][i]) < 1e-14 * (std::abs(m_T[i][i]) + std::abs(m_T[i + 1][i + 1]))) {
            /* 1x1块: 实特征值 */
            eigs.append(std::complex<double>(m_T[i][i], 0.0));
            i++;
        } else {
            /* 2x2块: 复共轭特征值对 */
            double a = m_T[i][i];
            double b = m_T[i][i + 1];
            double c = m_T[i + 1][i];
            double d = m_T[i + 1][i + 11];
            double trace = a + d;
            double det = a * d - b * c;
            double disc = trace * trace - 4.0 * det;
            double sqrtDisc = std::sqrt(std::abs(disc));

            if (disc >= 0) {
                eigs.append(std::complex<double>((trace + sqrtDisc) / 2.0, 0.0));
                eigs.append(std::complex<double>((trace - sqrtDisc) / 2.0, 0.0));
            } else {
                eigs.append(std::complex<double>(trace / 2.0, sqrtDisc / 2.0));
                eigs.append(std::complex<double>(trace / 2.0, -sqrtDisc / 2.0));
            }
            i += 2;
        }
    }

    return eigs;
}

/** @brief 重置统计信息 */
void SchurDecomp6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
