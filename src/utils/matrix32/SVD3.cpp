/**
 * @file SVD3.cpp
 * @brief SVD分解增强实现 — 双对角化/Golub-Kahan步/截断SVD/伪逆
 * @author EmbedDebug Team
 * @date 2026-06-05
 */

#include "utils/matrix32/SVD3.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>
#include <vector>

/** @brief 构造函数 @param parent 父对象 */
SVD3::SVD3(QObject* parent)
    : QObject(parent)
{
}

/** @brief 辅助函数: 访问矩阵A[i][j](行优先存储) */
static inline double& mat(QVector<double>& A, int rows, int /*cols*/,
                           int i, int j)
{
    return A[i + j * rows];
}

/** @brief 辅助函数: 访问矩阵A[i][j](const版本) */
static inline double matc(const QVector<double>& A, int rows, int /*cols*/,
                           int i, int j)
{
    return A[i + j * rows];
}

/** @brief Householder变换: 将向量x映射到e1方向
 *  @param x 向量数据(就地修改)
 *  @param n 向量长度
 *  @param stride 步长
 *  @return Householder系数beta
 */
static double householder(QVector<double>& x, int start, int n, int stride)
{
    double sigma = 0.0;
    for (int i = 1; i < n; ++i) {
        double xi = x[start + i * stride];
        sigma += xi * xi;
    }
    if (sigma < 1e-30) return 0.0;

    double x0 = x[start];
    double mu = qSqrt(x0 * x0 + sigma);
    double v0 = (x0 <= 0.0) ? x0 - mu : -sigma / (x0 + mu);
    double beta = 2.0 * v0 * v0 / (sigma + v0 * v0);
    x[start] = v0;
    for (int i = 1; i < n; ++i) {
        x[start + i * stride] /= v0;
    }
    return beta;
}

/** @brief 双对角化: 将A分解为 U * B * V^T
 *  其中B为上双对角矩阵，U和V为正交矩阵
 *  @param A 输入矩阵(就地修改为B)
 *  @param m 行数
 *  @param n 列数
 *  @param U 左正交矩阵
 *  @param V 右正交矩阵
 */
void SVD3::bidiagonalize(QVector<double>& A, int m, int n,
                          QVector<double>& U, QVector<double>& V)
{
    /* 初始化U=I, V=I */
    U.fill(0.0, m * m);
    V.fill(0.0, n * n);
    for (int i = 0; i < m; ++i) U[i + i * m] = 1.0;
    for (int i = 0; i < n; ++i) V[i + i * n] = 1.0;

    int minDim = qMin(m, n);
    for (int k = 0; k < minDim; ++k) {
        /* 左Householder: 消除A[k+1:m, k] */
        if (k < m - 1) {
            int len = m - k;
            QVector<double> v(len, 0.0);
            for (int i = 0; i < len; ++i) v[i] = mat(A, m, n, k + i, k);
            double beta = householder(v, 0, len, 1);
            if (beta > 1e-30) {
                /* 更新A: A[k:m, k:n] -= beta * v * (v^T * A[k:m, k:n]) */
                for (int j = k; j < n; ++j) {
                    double dot = 0.0;
                    for (int i = 0; i < len; ++i) {
                        dot += v[i] * mat(A, m, n, k + i, j);
                    }
                    for (int i = 0; i < len; ++i) {
                        mat(A, m, n, k + i, j) -= beta * v[i] * dot;
                    }
                }
                /* 更新U: U[:, k:m] -= beta * (U[:, k:m] * v) * v^T */
                for (int i = 0; i < m; ++i) {
                    double dot = 0.0;
                    for (int j = 0; j < len; ++j) {
                        dot += matc(U, m, m, i, k + j) * v[j];
                    }
                    for (int j = 0; j < len; ++j) {
                        mat(U, m, m, i, k + j) -= beta * dot * v[j];
                    }
                }
            }
        }

        /* 右Householder: 消除A[k, k+2:n] */
        if (k < n - 2) {
            int len = n - k - 1;
            QVector<double> v(len, 0.0);
            for (int i = 0; i < len; ++i) {
                v[i] = mat(A, m, n, k, k + 1 + i);
            }
            double beta = householder(v, 0, len, 1);
            if (beta > 1e-30) {
                /* 更新A: A[k:m, k+1:n] -= beta * (A[k:m, k+1:n]*v) * v^T */
                for (int i = k; i < m; ++i) {
                    double dot = 0.0;
                    for (int j = 0; j < len; ++j) {
                        dot += mat(A, m, n, i, k + 1 + j) * v[j];
                    }
                    for (int j = 0; j < len; ++j) {
                        mat(A, m, n, i, k + 1 + j) -= beta * dot * v[j];
                    }
                }
                /* 更新V: V[:, k+1:n] -= beta * (V[:, k+1:n]*v) * v^T */
                for (int i = 0; i < n; ++i) {
                    double dot = 0.0;
                    for (int j = 0; j < len; ++j) {
                        dot += matc(V, n, n, i, k + 1 + j) * v[j];
                    }
                    for (int j = 0; j < len; ++j) {
                        mat(V, n, n, i, k + 1 + j) -= beta * dot * v[j];
                    }
                }
            }
        }
    }
}

/** @brief Golub-Kahan SVD迭代步(隐式QR位移)
 *  对双对角矩阵的对角线d和上对角线e进行迭代
 *  @param d 对角线元素
 *  @param e 上对角线元素
 *  @param U 左正交矩阵(累积旋转)
 *  @param V 右正交矩阵(累积旋转)
 *  @param m 行数
 *  @param n 列数
 */
void SVD3::golubKahanStep(QVector<double>& d, QVector<double>& e,
                            QVector<double>& U, QVector<double>& V,
                            int m, int n)
{
    int N = d.size();
    if (N < 2) return;

    /* 寻找底部非零上对角线元素 */
    int q = N - 1;
    while (q > 0 && qAbs(e[q - 1]) < 1e-14 * (qAbs(d[q - 1]) + qAbs(d[q]))) {
        e[q - 1] = 0.0;
        --q;
    }
    if (q == 0) return;

    /* 寻找顶部边界 */
    int p = q - 1;
    while (p > 0 && qAbs(e[p - 1]) >= 1e-14 * (qAbs(d[p - 1]) + qAbs(d[p]))) {
        --p;
    }

    /* 计算Wilkinson位移 */
    double dd = d[q - 1], ee = e[q - 1], ff = d[q];
    double dm = (dd + ff) * 0.5;
    double dm2 = dm * dm;
    double dn2 = dd * ff - ee * ee;
    double mu = dm2 + dn2;
    double delta = dm2 - dn2;
    double disc = qSqrt(qAbs(delta * delta + 4.0 * ee * ee * dm2));
    double lambda = (delta >= 0) ? dm + disc : dm - disc;
    if (qAbs(dm - lambda) > qAbs(dm + lambda)) {
        lambda = dm + (delta >= 0 ? disc : -disc);
    }

    /* 追赶法消除上对角线 */
    double x = d[p] * d[p] - lambda;
    double z = d[p] * e[p];
    for (int k = p; k < q; ++k) {
        /* 右侧Givens旋转 */
        double r = qSqrt(x * x + z * z);
        if (r < 1e-30) { r = 1e-30; }
        double c = x / r, s = z / r;

        /* 更新V */
        for (int i = 0; i < n; ++i) {
            double v1 = matc(V, n, n, i, k);
            double v2 = (k + 1 < n) ? matc(V, n, n, i, k + 1) : 0.0;
            mat(V, n, n, i, k) = c * v1 + s * v2;
            if (k + 1 < n) mat(V, n, n, i, k + 1) = -s * v1 + c * v2;
        }

        /* 更新双对角元素 */
        double dk = d[k];
        double ek = e[k];
        if (k + 1 < N) {
            double dk1 = d[k + 1];
            d[k] = c * c * dk + 2.0 * c * s * ek + s * s * dk1;
            e[k] = c * s * (dk1 - dk) + (c * c - s * s) * ek;
            if (k + 1 < q) {
                x = e[k];
                z = s * e[k + 1];
                e[k + 1] = c * e[k + 1];
            }
        }

        /* 左侧Givens旋转 */
        if (k + 1 < N && qAbs(e[k]) > 1e-30) {
            r = qSqrt(d[k] * d[k] + e[k] * e[k]);
            c = d[k] / r;
            s = e[k] / r;
            d[k] = r;
            e[k] = 0.0;

            for (int i = 0; i < m; ++i) {
                double u1 = matc(U, m, m, i, k);
                double u2 = (k + 1 < m) ? matc(U, m, m, i, k + 1) : 0.0;
                mat(U, m, m, i, k) = c * u1 + s * u2;
                if (k + 1 < m) mat(U, m, m, i, k + 1) = -s * u1 + c * u2;
            }

            if (k + 1 < N) {
                d[k + 1] = c * d[k + 1];
            }
        }
    }
}

/** @brief 对矩阵进行SVD分解: A = U * diag(sigma) * V^T
 *  @param matrix 行优先输入矩阵(展开为一维)
 *  @param rows 行数
 *  @param cols 列数
 */
void SVD3::decompose(const QVector<double>& matrix, int rows, int cols)
{
    QElapsedTimer timer;
    timer.start();

    m_rows = rows;
    m_cols = cols;
    int m = rows, n = cols;
    int minDim = qMin(m, n);

    if (matrix.size() < m * n) {
        m_U.clear(); m_sigma.clear(); m_V.clear();
        return;
    }

    /* 复制输入矩阵 */
    QVector<double> A = matrix;

    /* 双对角化 */
    QVector<double> U, V;
    bidiagonalize(A, m, n, U, V);

    /* 提取双对角矩阵的对角线和上对角线 */
    QVector<double> d(minDim), e(minDim - 1, 0.0);
    for (int i = 0; i < minDim; ++i) {
        d[i] = mat(A, m, n, i, i);
    }
    for (int i = 0; i < minDim - 1; ++i) {
        e[i] = mat(A, m, n, i, i + 1);
    }

    /* Golub-Kahan迭代 */
    int maxIter = 100 * minDim;
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 检查收敛 */
        bool converged = true;
        for (int i = 0; i < minDim - 1; ++i) {
            if (qAbs(e[i]) > 1e-14 * (qAbs(d[i]) + qAbs(d[i + 1]))) {
                converged = false;
                break;
            }
        }
        if (converged) break;
        golubKahanStep(d, e, U, V, m, n);
    }

    /* 确保奇异值非负 */
    m_sigma = d;
    for (int i = 0; i < minDim; ++i) {
        if (m_sigma[i] < 0.0) {
            m_sigma[i] = -m_sigma[i];
            for (int j = 0; j < m; ++j) {
                mat(U, m, m, j, i) = -matc(U, m, m, j, i);
            }
        }
    }

    /* 按奇异值降序排列 */
    for (int i = 0; i < minDim - 1; ++i) {
        for (int j = i + 1; j < minDim; ++j) {
            if (m_sigma[j] > m_sigma[i]) {
                std::swap(m_sigma[i], m_sigma[j]);
                for (int r = 0; r < m; ++r) {
                    std::swap(mat(U, m, m, r, i), mat(U, m, m, r, j));
                }
                for (int r = 0; r < n; ++r) {
                    std::swap(mat(V, n, n, r, i), mat(V, n, n, r, j));
                }
            }
        }
    }

    m_U = U;
    m_V = V;

    m_stats.totalDecompositions++;
    qint64 elapsed = timer.elapsed();
    m_timeSum += static_cast<double>(elapsed);
    m_stats.avgProcessingTimeMs = (m_stats.totalDecompositions > 0) ?
        m_timeSum / m_stats.totalDecompositions : 0.0;
    emit decompositionComplete(m, n, rank());
}

/** @brief 利用SVD求解线性方程组Ax=b @param b 右端向量 @return 解向量x */
QVector<double> SVD3::solve(const QVector<double>& b) const
{
    if (m_sigma.isEmpty() || m_U.isEmpty() || m_V.isEmpty()) {
        return QVector<double>();
    }

    int m = m_rows, n = m_cols;
    int minDim = qMin(m, n);

    /* x = V * diag(1/sigma) * U^T * b */
    /* 计算 U^T * b */
    QVector<double> utb(minDim, 0.0);
    for (int i = 0; i < minDim; ++i) {
        for (int j = 0; j < m; ++j) {
            utb[i] += matc(m_U, m, m, j, i) * ((j < b.size()) ? b[j] : 0.0);
        }
    }

    /* 乘以 1/sigma */
    for (int i = 0; i < minDim; ++i) {
        if (qAbs(m_sigma[i]) > 1e-14) {
            utb[i] /= m_sigma[i];
        } else {
            utb[i] = 0.0;
        }
    }

    /* 乘以 V */
    QVector<double> x(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < minDim; ++j) {
            x[i] += matc(m_V, n, n, i, j) * utb[j];
        }
    }

    return x;
}

/** @brief 计算伪逆矩阵 A+ = V * diag(1/sigma) * U^T @return 伪逆矩阵(展开) */
QVector<double> SVD3::pseudoInverse() const
{
    if (m_sigma.isEmpty()) return QVector<double>();

    int m = m_rows, n = m_cols;
    int minDim = qMin(m, n);

    /* A+ = V * S+ * U^T，其中S+为sigma倒序且取倒数 */
    QVector<double> result(n * m, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < m; ++j) {
            double sum = 0.0;
            for (int k = 0; k < minDim; ++k) {
                if (qAbs(m_sigma[k]) > 1e-14) {
                    sum += matc(m_V, n, n, i, k) * (1.0 / m_sigma[k])
                         * matc(m_U, m, m, j, k);
                }
            }
            result[i + j * n] = sum;
        }
    }

    return result;
}

/** @brief 截断SVD: 保留前r个最大奇异值的低秩近似
 *  @param r 保留的秩
 *  @return 低秩近似矩阵(展开)
 */
QVector<double> SVD3::truncatedSVD(int r) const
{
    if (m_sigma.isEmpty()) return QVector<double>();

    int m = m_rows, n = m_cols;
    int minDim = qMin(m, n);
    r = qBound(1, r, minDim);

    QVector<double> result(m * n, 0.0);
    for (int k = 0; k < r; ++k) {
        for (int i = 0; i < m; ++i) {
            for (int j = 0; j < n; ++j) {
                result[i + j * m] += m_sigma[k]
                    * matc(m_U, m, m, i, k)
                    * matc(m_V, n, n, j, k);
            }
        }
    }

    return result;
}

/** @brief 计算矩阵条件数(最大/最小奇异值之比) @return 条件数 */
double SVD3::conditionNumber() const
{
    if (m_sigma.size() < 2) return 1.0;

    double maxS = m_sigma[0];
    double minS = m_sigma[0];
    for (double s : m_sigma) {
        if (s > maxS) maxS = s;
        if (s < minS && s > 1e-30) minS = s;
    }

    return (minS > 1e-30) ? maxS / minS : 1e30;
}

/** @brief 计算矩阵的数值秩 @param tol 容差 @return 秩 */
int SVD3::rank(double tol) const
{
    if (m_sigma.isEmpty()) return 0;

    double maxS = m_sigma[0];
    if (maxS < 1e-30) return 0;

    double threshold = tol * maxS;
    int r = 0;
    for (double s : m_sigma) {
        if (s > threshold) ++r;
    }
    return r;
}

/** @brief 重置所有统计计数器 */
void SVD3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
