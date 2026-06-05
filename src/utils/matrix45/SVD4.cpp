/**
 * @file SVD4.cpp
 * @brief SVD4实现 — 双边Jacobi+条件数估计
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix45/SVD4.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <limits>

/**
 * @brief 构造函数，初始化默认参数
 * @param parent 父对象
 */
SVD4::SVD4(QObject* parent)
    : QObject(parent)
{
    setObjectName(QStringLiteral("SVD4"));
}

/**
 * @brief 对矩阵执行奇异值分解 A = U * Sigma * V^T
 *
 * 使用双边Jacobi旋转方法。每次Jacobi扫描将一对行/列的
 * 非对角元素归零，直到收敛。
 *
 * @param A 输入矩阵（行优先，rows*cols个元素）
 * @param rows 行数
 * @param cols 列数
 * @return 分解是否成功
 */
bool SVD4::decompose(const QVector<double>& A, int rows, int cols)
{
    QElapsedTimer timer;
    timer.start();

    if (rows <= 0 || cols <= 0 || A.size() < rows * cols) return false;

    m_rows = rows;
    m_cols = cols;

    /* 初始化 U = I (rows*rows) */
    m_U.resize(rows * rows, 0.0);
    for (int i = 0; i < rows; ++i) m_U[i * rows + i] = 1.0;

    /* 初始化 V = I (cols*cols) */
    m_V.resize(cols * cols, 0.0);
    for (int i = 0; i < cols; ++i) m_V[i * cols + i] = 1.0;

    /* S = A^T * A (cols*cols 对称矩阵) */
    /* 改用 S = A^T * A 的 Jacobi 方法 */
    QVector<double> S(cols * cols, 0.0);
    for (int i = 0; i < cols; ++i) {
        for (int j = 0; j < cols; ++j) {
            double sum = 0.0;
            for (int k = 0; k < rows; ++k) {
                sum += A[k * cols + i] * A[k * cols + j];
            }
            S[i * cols + j] = sum;
        }
    }

    /* 双边Jacobi迭代 */
    const int maxSweeps = 100;
    for (int sweep = 0; sweep < maxSweeps; ++sweep) {
        double offNorm = 0.0;
        for (int i = 0; i < cols; ++i) {
            for (int j = i + 1; j < cols; ++j) {
                offNorm += S[i * cols + j] * S[i * cols + j];
            }
        }
        if (offNorm < 1e-20) break;

        jacobiSweep(S, m_V, m_V, cols, cols);
    }

    /* 提取奇异值（S的对角元素的平方根） */
    m_sigma.resize(qMin(rows, cols));
    for (int i = 0; i < m_sigma.size(); ++i) {
        m_sigma[i] = qSqrt(qMax(0.0, S[i * cols + i]));
    }

    /* 排序奇异值（降序） */
    QVector<int> order(m_sigma.size());
    for (int i = 0; i < order.size(); ++i) order[i] = i;
    std::sort(order.begin(), order.end(), [&](int a, int b) {
        return m_sigma[a] > m_sigma[b];
    });

    QVector<double> sortedSigma(m_sigma.size());
    QVector<double> sortedV(m_V.size(), 0.0);
    for (int i = 0; i < order.size(); ++i) {
        sortedSigma[i] = m_sigma[order[i]];
        for (int j = 0; j < cols; ++j) {
            sortedV[j * cols + i] = m_V[j * cols + order[i]];
        }
    }
    m_sigma = sortedSigma;
    m_V = sortedV;

    /* 计算 U = A * V * Sigma^{-1} */
    for (int i = 0; i < rows; ++i) {
        for (int j = 0; j < m_sigma.size(); ++j) {
            double sum = 0.0;
            for (int k = 0; k < cols; ++k) {
                sum += A[i * cols + k] * m_V[k * cols + j];
            }
            m_U[i * rows + j] = (m_sigma[j] > 1e-15) ? sum / m_sigma[j] : 0.0;
        }
    }

    m_stats.totalDecompositions++;
    m_stats.matrixRows = rows;

    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    int r = rank();
    emit decompositionCompleted(rows, cols, r);
    return true;
}

/**
 * @brief 计算矩阵的秩
 *
 * 统计大于给定容差的奇异值数量。
 *
 * @param tol 容差阈值
 * @return 矩阵的秩
 */
int SVD4::rank(double tol) const
{
    if (m_sigma.isEmpty()) return 0;
    double maxSV = *std::max_element(m_sigma.begin(), m_sigma.end());
    double threshold = tol * maxSV * qMax(m_rows, m_cols);
    int r = 0;
    for (double s : m_sigma) {
        if (s > threshold) r++;
    }
    return r;
}

/**
 * @brief 求解线性方程组 Ax = b
 *
 * 使用SVD的伪逆解：x = V * Sigma^{-1} * U^T * b
 *
 * @param b 右侧向量（m_rows个元素）
 * @return 解向量（m_cols个元素）
 */
QVector<double> SVD4::solve(const QVector<double>& b) const
{
    if (b.size() != m_rows || m_sigma.isEmpty()) return {};

    /* U^T * b */
    QVector<double> utb(m_sigma.size(), 0.0);
    for (int i = 0; i < m_sigma.size(); ++i) {
        for (int j = 0; j < m_rows; ++j) {
            utb[i] += m_U[j * m_rows + i] * b[j];
        }
    }

    /* Sigma^{-1} * U^T * b */
    double maxSV = m_sigma[0];
    for (int i = 0; i < m_sigma.size(); ++i) {
        if (m_sigma[i] > maxSV * 1e-10) {
            utb[i] /= m_sigma[i];
        } else {
            utb[i] = 0.0;
        }
    }

    /* V * result */
    QVector<double> x(m_cols, 0.0);
    for (int i = 0; i < m_cols; ++i) {
        for (int j = 0; j < m_sigma.size(); ++j) {
            x[i] += m_V[i * m_cols + j] * utb[j];
        }
    }

    return x;
}

/**
 * @brief 计算条件数
 *
 * 条件数 = 最大奇异值 / 最小奇异值。
 * 条件数大表示矩阵接近奇异。
 *
 * @return 条件数
 */
double SVD4::conditionNumber() const
{
    if (m_sigma.isEmpty()) return 0.0;
    double maxSV = m_sigma[0];
    double minSV = m_sigma.last();
    if (minSV < 1e-15) return std::numeric_limits<double>::max();
    return maxSV / minSV;
}

/**
 * @brief 计算Moore-Penrose伪逆
 *
 * A^+ = V * Sigma^+ * U^T
 * 其中 Sigma^+ 将非零奇异值取倒数。
 *
 * @return 伪逆矩阵（m_cols * m_rows）
 */
QVector<double> SVD4::pseudoInverse() const
{
    if (m_sigma.isEmpty()) return {};

    /* Sigma^+ * U^T (min(m,n) * m_rows) */
    QVector<double> sigmaUt(m_sigma.size() * m_rows, 0.0);
    for (int i = 0; i < m_sigma.size(); ++i) {
        double invS = (m_sigma[i] > m_sigma[0] * 1e-10) ? 1.0 / m_sigma[i] : 0.0;
        for (int j = 0; j < m_rows; ++j) {
            sigmaUt[i * m_rows + j] = invS * m_U[j * m_rows + i];
        }
    }

    /* V * (Sigma^+ * U^T) (m_cols * m_rows) */
    QVector<double> pinv(m_cols * m_rows, 0.0);
    for (int i = 0; i < m_cols; ++i) {
        for (int j = 0; j < m_rows; ++j) {
            double sum = 0.0;
            for (int k = 0; k < m_sigma.size(); ++k) {
                sum += m_V[i * m_cols + k] * sigmaUt[k * m_rows + j];
            }
            pinv[i * m_rows + j] = sum;
        }
    }

    return pinv;
}

/**
 * @brief Jacobi扫描 — 消除一对非对角元素
 *
 * 对对称矩阵S执行Jacobi旋转，每次处理一对(p,q)，
 * 将S[p][q]归零并更新特征向量矩阵。
 *
 * @param S 对称矩阵（会被修改）
 * @param U 左变换矩阵
 * @param V 右变换矩阵
 * @param m 行数
 * @param n 列数
 */
void SVD4::jacobiSweep(QVector<double>& S, QVector<double>& U,
                         QVector<double>& V, int m, int n)
{
    for (int p = 0; p < n; ++p) {
        for (int q = p + 1; q < n; ++q) {
            rotate(S, U, V, p, q, m, n);
        }
    }
}

/**
 * @brief 单次Jacobi旋转
 *
 * 计算Givens旋转角并应用于矩阵S和特征向量。
 *
 * @param S 对称矩阵
 * @param U 左变换矩阵
 * @param V 右变换矩阵
 * @param p 行索引1
 * @param q 行索引2
 * @param m 行数
 * @param n 列数
 */
void SVD4::rotate(QVector<double>& S, QVector<double>& U,
                   QVector<double>& V, int p, int q, int m, int n)
{
    double spq = S[p * n + q];
    if (qFabs(spq) < 1e-15) return;

    double spp = S[p * n + p];
    double sqq = S[q * n + q];

    double tau = (sqq - spp) / (2.0 * spq);
    double t = (tau >= 0) ? 1.0 / (tau + qSqrt(1.0 + tau * tau))
                          : -1.0 / (-tau + qSqrt(1.0 + tau * tau));

    double c = 1.0 / qSqrt(1.0 + t * t);
    double s = t * c;

    /* 更新S */
    S[p * n + p] = c * c * spp - 2.0 * c * s * spq + s * s * sqq;
    S[q * n + q] = s * s * spp + 2.0 * c * s * spq + c * c * sqq;
    S[p * n + q] = 0.0;
    S[q * n + p] = 0.0;

    for (int r = 0; r < n; ++r) {
        if (r == p || r == q) continue;
        double srp = S[r * n + p];
        double srq = S[r * n + q];
        S[r * n + p] = c * srp - s * srq;
        S[p * n + r] = S[r * n + p];
        S[r * n + q] = s * srp + c * srq;
        S[q * n + r] = S[r * n + q];
    }

    /* 更新V */
    for (int r = 0; r < n; ++r) {
        double vrp = V[r * n + p];
        double vrq = V[r * n + q];
        V[r * n + p] = c * vrp - s * vrq;
        V[r * n + q] = s * vrp + c * vrq;
    }
}

/**
 * @brief 重置所有统计数据
 */
void SVD4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
