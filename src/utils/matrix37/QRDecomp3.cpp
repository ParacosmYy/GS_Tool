/**
 * @file QRDecomp3.cpp
 * @brief QR分解增强实现 — Householder变换/列主元选择/最小二乘求解
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix37/QRDecomp3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <numeric>

/**
 * @brief 构造函数
 * @param parent 父对象
 */
QRDecomp3::QRDecomp3(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("QRDecomp3"));
}

/**
 * @brief 执行带列主元的QR分解
 *
 * 使用Householder变换将矩阵A分解为 QR = AP^T，
 * 其中Q为正交矩阵，R为上三角矩阵，P为列置换矩阵。
 * 列主元选择保证数值稳定性。
 *
 * 矩阵按行主序存储: matrix[row * cols + col]
 *
 * @param matrix 输入矩阵（行主序）
 * @param rows 行数
 * @param cols 列数
 */
void QRDecomp3::decompose(const QVector<double>& matrix, int rows, int cols)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = rows;
    m_cols = cols;
    int m = rows;
    int n = cols;
    int minMN = qMin(m, n);

    /* 将输入矩阵复制到QR中（QR = A，后续原地变换为R） */
    m_QR = matrix;
    m_tau.resize(minMN);
    m_pivot.resize(n);
    std::iota(m_pivot.begin(), m_pivot.end(), 0);

    /* 列范数缓存 */
    QVector<double> colNorm(n);
    for (int j = 0; j < n; ++j) {
        double sum = 0.0;
        for (int i = 0; i < m; ++i) {
            double v = m_QR[i * n + j];
            sum += v * v;
        }
        colNorm[j] = sum;
    }

    /* 逐列执行Householder变换 */
    for (int k = 0; k < minMN; ++k) {
        /* 列主元选择: 找范数最大的列 */
        int maxCol = k;
        double maxNorm = colNorm[k];
        for (int j = k + 1; j < n; ++j) {
            if (colNorm[j] > maxNorm) {
                maxNorm = colNorm[j];
                maxCol = j;
            }
        }

        /* 交换列 */
        if (maxCol != k) {
            for (int i = 0; i < m; ++i) {
                std::swap(m_QR[i * n + k], m_QR[i * n + maxCol]);
            }
            std::swap(m_pivot[k], m_pivot[maxCol]);
            std::swap(colNorm[k], colNorm[maxCol]);
        }

        /* 计算Householder向量 v = x - ||x|| * e1 */
        double normX = 0.0;
        for (int i = k; i < m; ++i) {
            double v = m_QR[i * n + k];
            normX += v * v;
        }
        normX = qSqrt(normX);

        if (normX < 1e-15) {
            m_tau[k] = 0.0;
            continue;
        }

        /* 选择符号使v[0]的模增大，提高数值稳定性 */
        double x0 = m_QR[k * n + k];
        double sign = (x0 >= 0.0) ? 1.0 : -1.0;
        double alpha = -sign * normX;

        /* v = x - alpha * e1 */
        m_QR[k * n + k] = x0 - alpha;

        /* tau = 2 / (v^T * v) */
        double vtv = (x0 - alpha) * (x0 - alpha);
        for (int i = k + 1; i < m; ++i) {
            double v = m_QR[i * n + k];
            vtv += v * v;
        }

        if (vtv < 1e-30) {
            m_tau[k] = 0.0;
            continue;
        }

        m_tau[k] = 2.0 * (x0 - alpha) * (x0 - alpha) / vtv;

        /* 归一化v的首元素 */
        double v0 = m_QR[k * n + k];
        for (int i = k + 1; i < m; ++i) {
            m_QR[i * n + k] /= v0;
        }
        m_QR[k * n + k] = 1.0;

        /* 应用Householder变换: R[k:m, k+1:n] -= tau * v * (v^T * R[k:m, k+1:n]) */
        for (int j = k + 1; j < n; ++j) {
            double dot = m_QR[k * n + j]; /* v[0]=1 */
            for (int i = k + 1; i < m; ++i) {
                dot += m_QR[i * n + k] * m_QR[i * n + j];
            }
            dot *= m_tau[k];
            m_QR[k * n + j] -= dot;
            for (int i = k + 1; i < m; ++i) {
                m_QR[i * n + j] -= m_QR[i * n + k] * dot;
            }
        }

        /* 更新后续列的范数 */
        for (int j = k + 1; j < n; ++j) {
            double v = m_QR[k * n + j];
            colNorm[j] -= v * v;
            if (colNorm[j] < 0.0) colNorm[j] = 0.0;
        }
    }

    double elapsed = timer.elapsed();
    m_timeSum += elapsed;
    m_stats.totalDecompositions++;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    int r = rank();
    emit decompositionComplete(m_rows, m_cols, r);
}

/**
 * @brief 利用QR分解求解线性方程组 Ax = b
 *
 * Q * R * P * x = b → R * P * x = Q^T * b → 回代
 *
 * @param rhs 右端向量（长度=rows）
 * @return 解向量（长度=cols）
 */
QVector<double> QRDecomp3::solve(const QVector<double>& rhs) const
{
    if (m_QR.isEmpty()) return {};

    int m = m_rows;
    int n = m_cols;
    int minMN = qMin(m, n);

    /* 计算 Q^T * b = H_{minMN-1} * ... * H_0 * b */
    QVector<double> qtb = rhs;
    for (int k = 0; k < minMN; ++k) {
        if (m_tau[k] == 0.0) continue;

        /* 应用H_k: qtb[k:m] -= tau * v * (v^T * qtb[k:m]) */
        double dot = qtb[k]; /* v[0] = 1 */
        for (int i = k + 1; i < m; ++i) {
            dot += m_QR[i * n + k] * qtb[i];
        }
        dot *= m_tau[k];

        qtb[k] -= dot;
        for (int i = k + 1; i < m; ++i) {
            qtb[i] -= m_QR[i * n + k] * dot;
        }
    }

    /* 回代求解 R * z = qtb[0:minMN] */
    QVector<double> z(n, 0.0);
    for (int i = minMN - 1; i >= 0; --i) {
        double sum = qtb[i];
        for (int j = i + 1; j < minMN; ++j) {
            sum -= m_QR[i * n + j] * z[j];
        }
        double diag = m_QR[i * n + i];
        z[i] = (qFabs(diag) > 1e-15) ? sum / diag : 0.0;
    }

    /* 反置换: x[pivot] = z */
    QVector<double> x(n, 0.0);
    for (int i = 0; i < n; ++i) {
        if (i < m_pivot.size()) {
            x[m_pivot[i]] = z[i];
        }
    }

    return x;
}

/**
 * @brief 提取正交矩阵Q
 * @return Q矩阵（行主序，rows x min(rows,cols)）
 */
QVector<double> QRDecomp3::matrixQ() const
{
    int m = m_rows;
    int n = m_cols;
    int minMN = qMin(m, n);

    /* Q = I - H_0 - H_1 - ... 从后往前累积 */
    QVector<double> Q(m * minMN, 0.0);
    for (int i = 0; i < m; ++i) {
        for (int j = 0; j < minMN; ++j) {
            Q[i * minMN + j] = (i == j) ? 1.0 : 0.0;
        }
    }

    for (int k = minMN - 1; k >= 0; --k) {
        if (m_tau[k] == 0.0) continue;

        /* 对Q的k行以后、k列以后应用H_k */
        for (int j = k; j < minMN; ++j) {
            double dot = Q[k * minMN + j];
            for (int i = k + 1; i < m; ++i) {
                dot += m_QR[i * n + k] * Q[i * minMN + j];
            }
            dot *= m_tau[k];

            Q[k * minMN + j] -= dot;
            for (int i = k + 1; i < m; ++i) {
                Q[i * minMN + j] -= m_QR[i * n + k] * dot;
            }
        }
    }

    return Q;
}

/**
 * @brief 提取上三角矩阵R
 * @return R矩阵（行主序，min(rows,cols) x cols）
 */
QVector<double> QRDecomp3::matrixR() const
{
    int m = m_rows;
    int n = m_cols;
    int minMN = qMin(m, n);

    QVector<double> R(minMN * n, 0.0);
    for (int i = 0; i < minMN; ++i) {
        for (int j = i; j < n; ++j) {
            R[i * n + j] = m_QR[i * n + j];
        }
    }
    return R;
}

/**
 * @brief 计算矩阵的秩
 * @param tol 奇异值容差
 * @return 秩
 */
int QRDecomp3::rank(double tol) const
{
    if (m_QR.isEmpty()) return 0;

    int minMN = qMin(m_rows, m_cols);
    int r = 0;

    for (int i = 0; i < minMN; ++i) {
        if (qFabs(m_QR[i * m_cols + i]) > tol) {
            r++;
        }
    }
    return r;
}

/**
 * @brief 计算条件数估计
 *
 * 使用R对角元素的 |max|/|min| 作为条件数的粗略估计。
 *
 * @return 条件数估计值
 */
double QRDecomp3::conditionNumber() const
{
    if (m_QR.isEmpty()) return 0.0;

    int minMN = qMin(m_rows, m_cols);
    double maxDiag = 0.0;
    double minDiag = 1e18;

    for (int i = 0; i < minMN; ++i) {
        double d = qFabs(m_QR[i * m_cols + i]);
        if (d > maxDiag) maxDiag = d;
        if (d < minDiag && d > 1e-15) minDiag = d;
    }

    if (minDiag < 1e-15) return 1e18;
    return maxDiag / minDiag;
}

/**
 * @brief 最小二乘求解
 *
 * 对超定方程组 Ax ≈ b，求解 min ||Ax - b||^2。
 * 等价于 R * P * x = Q^T * b 的最小范数解。
 *
 * @param rhs 右端向量
 * @return 最小二乘解
 */
QVector<double> QRDecomp3::leastSquares(const QVector<double>& rhs) const
{
    /* 最小二乘与solve使用相同的QR回代 */
    return solve(rhs);
}

/**
 * @brief 重置所有累积统计信息
 */
void QRDecomp3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
