/**
 * @file SVD5.cpp
 * @brief 奇异值分解（SVD）实现
 *
 * 实现基于双边正交化和Golub-Kahan对角化的奇异值分解。
 * SVD将任意 m x n 矩阵 A 分解为 A = U * Sigma * V^T，其中:
 * - U: m x m 正交矩阵（左奇异向量）
 * - Sigma: m x n 对角矩阵（奇异值，降序排列）
 * - V: n x n 正交矩阵（右奇异向量）
 *
 * SVD广泛应用于: 降维（PCA）、矩阵近似、线性最小二乘、条件数估计等。
 *
 * @author EmbedDebug Team
 * @version 1.0
 * @date 2026-06-05
 */

#include "utils/matrix56/SVD5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/**
 * @brief 构造函数
 * @param parent 父QObject对象指针
 */
SVD5::SVD5(QObject* parent)
    : QObject(parent)
    , m_rows(0)
    , m_cols(0)
    , m_timeSum(0.0)
{
}

/**
 * @brief 对矩阵执行奇异值分解
 *
 * 算法步骤:
 * 1. 双边正交化（Householder变换）将A转化为上双对角矩阵B
 * 2. 对B执行隐式QR迭代求奇异值
 * 3. 累积正交变换得到U和V
 *
 * @param A 输入矩阵，行优先存储（A[i*cols+j] = 原始矩阵第i行第j列）
 * @param rows 行数
 * @param cols 列数
 * @return true分解成功，false输入无效
 */
bool SVD5::decompose(const QVector<double>& A, int rows, int cols)
{
    QElapsedTimer timer;
    timer.start();

    if (A.size() < rows * cols || rows <= 0 || cols <= 0) {
        return false;
    }

    m_rows = rows;
    m_cols = cols;
    int m = rows;
    int n = cols;
    int minMN = qMin(m, n);

    /* 初始化U为单位矩阵 */
    m_U.resize(m * m, 0.0);
    for (int i = 0; i < m; ++i) {
        m_U[i * m + i] = 1.0;
    }

    /* 初始化V为单位矩阵 */
    m_V.resize(n * n, 0.0);
    for (int i = 0; i < n; ++i) {
        m_V[i * n + i] = 1.0;
    }

    /* 复制A到工作矩阵B（上双对角化目标） */
    QVector<double> B = A;

    /* 双边Householder正交化: A -> U^T * A * V = B（上双对角矩阵） */
    bidiagonalize(B, m_U, m_V, m, n);

    /* 提取对角线(d)和上对角线(e) */
    QVector<double> d(minMN, 0.0);     ///< 主对角线
    QVector<double> e(minMN, 0.0);     ///< 上对角线

    for (int i = 0; i < minMN; ++i) {
        d[i] = B[i * n + i];
    }
    for (int i = 0; i < minMN - 1; ++i) {
        e[i] = B[i * n + i + 1];
    }

    /* 隐式QR迭代求奇异值 */
    int maxIter = 100 * minMN;
    for (int iter = 0; iter < maxIter; ++iter) {
        /* 检查收敛: 小的上对角线元素可以置零 */
        bool converged = true;
        for (int i = 0; i < minMN - 1; ++i) {
            if (qAbs(e[i]) > 1e-14 * (qAbs(d[i]) + qAbs(d[i + 1]))) {
                converged = false;
                break;
            }
        }
        if (converged) {
            break;
        }

        /* 寻找最大未收敛子矩阵 [l, m-1] */
        int l = minMN - 1;
        while (l > 0 && qAbs(e[l - 1]) <= 1e-14 * (qAbs(d[l - 1]) + qAbs(d[l]))) {
            l--;
        }

        if (l == minMN - 1) {
            /* 1x1块已收敛 */
            continue;
        }

        /* Wilkinson位移 */
        double dd = d[minMN - 1];
        double ee = e[minMN - 2];
        double dm = d[minMN - 2];
        double em = (minMN > 2) ? e[minMN - 3] : 0.0;

        double a2 = dm * dm + ee * ee;
        double b2 = dd * dd + e[minMN - 1] * e[minMN - 1];

        /* 简化: 使用Givens旋转进行隐式QR步 */
        double x = d[l];
        double z = e[l];

        for (int k = l; k < minMN - 1; ++k) {
            /* 计算Givens旋转参数 */
            double r = qSqrt(x * x + z * z);
            double c = (qAbs(r) > 1e-30) ? x / r : 1.0;
            double s = (qAbs(r) > 1e-30) ? -z / r : 0.0;

            if (k > l) {
                e[k - 1] = r;
            }

            /* 旋转对角线元素 */
            double d1 = c * d[k] - s * e[k];
            double e1 = s * d[k] + c * e[k];
            double d2 = c * e[k] + s * d[k + 1];
            double e2 = -s * d[k + 1] + c * e[k];

            /* 实际上是2x2 SVD步 */
            d[k] = d1;
            e[k] = e2;
            d[k + 1] = qSqrt(d2 * d2 + e1 * e1);

            /* 更新V矩阵的对应列 */
            for (int i = 0; i < n; ++i) {
                double v1 = m_V[i * n + k];
                double v2 = m_V[i * n + k + 1];
                m_V[i * n + k] = c * v1 - s * v2;
                m_V[i * n + k + 1] = s * v1 + c * v2;
            }

            /* 更新U矩阵的对应列 */
            for (int i = 0; i < m; ++i) {
                double u1 = m_U[i * m + k];
                double u2 = m_U[i * m + k + 1];
                m_U[i * m + k] = c * u1 - s * u2;
                m_U[i * m + k + 1] = s * u1 + c * u2;
            }

            /* 准备下一步 */
            if (k < minMN - 2) {
                x = e1;
                z = 0.0;
            }
        }
    }

    /* 奇异值取绝对值并降序排列 */
    m_sigma.resize(minMN);
    for (int i = 0; i < minMN; ++i) {
        m_sigma[i] = qAbs(d[i]);
    }

    /* 降序排序（同时调整U和V的列顺序） */
    for (int i = 0; i < minMN - 1; ++i) {
        int maxIdx = i;
        for (int j = i + 1; j < minMN; ++j) {
            if (m_sigma[j] > m_sigma[maxIdx]) {
                maxIdx = j;
            }
        }
        if (maxIdx != i) {
            std::swap(m_sigma[i], m_sigma[maxIdx]);

            /* 交换U的第i列和第maxIdx列 */
            for (int r = 0; r < m; ++r) {
                std::swap(m_U[r * m + i], m_U[r * m + maxIdx]);
            }

            /* 交换V的第i列和第maxIdx列 */
            for (int r = 0; r < n; ++r) {
                std::swap(m_V[r * n + i], m_V[r * n + maxIdx]);
            }
        }
    }

    /* 更新统计信息 */
    double elapsed = timer.elapsed();
    m_stats.totalDecompositions++;
    m_stats.matrixRows = m * n;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalDecompositions;

    emit decompositionCompleted(m, n);
    return true;
}

/**
 * @brief 双边正交化
 *
 * 使用Householder变换将矩阵B转化为上双对角矩阵形式。
 * 左侧Householder变换累积到U中，右侧Householder变换累积到V中。
 *
 * @param B 工作矩阵（会被修改为上双对角矩阵）
 * @param U 左正交矩阵（累积变换）
 * @param V 右正交矩阵（累积变换）
 * @param m 行数
 * @param n 列数
 */
void SVD5::bidiagonalize(QVector<double>& B, QVector<double>& U,
                          QVector<double>& V, int m, int n)
{
    int minMN = qMin(m, n);

    for (int k = 0; k < minMN; ++k) {
        /* 左侧Householder: 消去第k列中第k行以下的元素 */
        double norm = 0.0;
        for (int i = k; i < m; ++i) {
            double val = B[i * n + k];
            norm += val * val;
        }
        norm = qSqrt(norm);

        if (norm > 1e-30) {
            double sign = (B[k * n + k] >= 0) ? 1.0 : -1.0;
            double alpha = -sign * norm;
            double r = qSqrt(0.5 * (alpha * alpha - B[k * n + k] * alpha));

            if (r > 1e-30) {
                QVector<double> v(m, 0.0);
                v[k] = (B[k * n + k] - alpha) / (2.0 * r);
                for (int i = k + 1; i < m; ++i) {
                    v[i] = B[i * n + k] / (2.0 * r);
                }

                /* B = (I - 2*v*v^T) * B */
                for (int j = k; j < n; ++j) {
                    double dot = 0.0;
                    for (int i = k; i < m; ++i) {
                        dot += v[i] * B[i * n + j];
                    }
                    for (int i = k; i < m; ++i) {
                        B[i * n + j] -= 2.0 * v[i] * dot;
                    }
                }

                /* 累积到U: U = U * (I - 2*v*v^T) */
                for (int j = 0; j < m; ++j) {
                    double dot = 0.0;
                    for (int i = k; i < m; ++i) {
                        dot += v[i] * U[j * m + i];
                    }
                    for (int i = k; i < m; ++i) {
                        U[j * m + i] -= 2.0 * v[i] * dot;
                    }
                }
            }
        }

        /* 右侧Householder: 消去第k行中第k+1列右侧的元素 */
        if (k < n - 2) {
            double norm2 = 0.0;
            for (int j = k + 1; j < n; ++j) {
                double val = B[k * n + j];
                norm2 += val * val;
            }
            norm2 = qSqrt(norm2);

            if (norm2 > 1e-30) {
                double sign = (B[k * n + k + 1] >= 0) ? 1.0 : -1.0;
                double alpha2 = -sign * norm2;
                double r2 = qSqrt(0.5 * (alpha2 * alpha2 - B[k * n + k + 1] * alpha2));

                if (r2 > 1e-30) {
                    QVector<double> v(n, 0.0);
                    v[k + 1] = (B[k * n + k + 1] - alpha2) / (2.0 * r2);
                    for (int j = k + 2; j < n; ++j) {
                        v[j] = B[k * n + j] / (2.0 * r2);
                    }

                    /* B = B * (I - 2*v*v^T) */
                    for (int i = 0; i < m; ++i) {
                        double dot = 0.0;
                        for (int j = k + 1; j < n; ++j) {
                            dot += v[j] * B[i * n + j];
                        }
                        for (int j = k + 1; j < n; ++j) {
                            B[i * n + j] -= 2.0 * v[j] * dot;
                        }
                    }

                    /* 累积到V: V = V * (I - 2*v*v^T) */
                    for (int i = 0; i < n; ++i) {
                        double dot = 0.0;
                        for (int j = k + 1; j < n; ++j) {
                            dot += v[j] * V[i * n + j];
                        }
                        for (int j = k + 1; j < n; ++j) {
                            V[i * n + j] -= 2.0 * v[j] * dot;
                        }
                    }
                }
            }
        }
    }
}

/**
 * @brief 计算矩阵的数值秩
 *
 * 统计大于给定容差的奇异值个数作为矩阵的数值秩。
 * 默认容差为 max(m,n) * sigma_max * eps。
 *
 * @param tol 容差阈值，奇异值小于此值视为零
 * @return 数值秩
 */
int SVD5::rank(double tol) const
{
    if (m_sigma.isEmpty()) {
        return 0;
    }

    double maxSigma = m_sigma[0];
    double defaultTol = qMax(m_rows, m_cols) * maxSigma * 2.2e-16;
    double threshold = (tol > 0) ? tol : defaultTol;

    int r = 0;
    for (double sv : m_sigma) {
        if (sv > threshold) {
            r++;
        }
    }

    return r;
}

/**
 * @brief 计算矩阵的条件数
 *
 * 条件数 = 最大奇异值 / 最小奇异值。
 * 条件数越大，矩阵越接近奇异，数值求解越不稳定。
 *
 * @return 条件数，若矩阵奇异则返回无穷大
 */
double SVD5::conditionNumber() const
{
    if (m_sigma.size() < 2) {
        return (m_sigma.isEmpty() || m_sigma[0] == 0.0)
            ? std::numeric_limits<double>::infinity() : 1.0;
    }

    double maxSV = m_sigma.first();
    double minSV = m_sigma.last();

    if (minSV < 1e-30) {
        return std::numeric_limits<double>::infinity();
    }

    return maxSV / minSV;
}

/**
 * @brief 重置所有统计计数器
 */
void SVD5::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
