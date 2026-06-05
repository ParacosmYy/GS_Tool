/**
 * @file LanczosEigen.cpp
 * @brief Lanczos算法实现 — 对称稀疏矩阵特征值求解
 */

#include "utils/lanczos/LanczosEigen.h"

#include <QElapsedTimer>
#include <QtMath>

#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
LanczosEigen::LanczosEigen(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 求解最大特征值 */
QVector<double> LanczosEigen::solve(const MatVecFunc& matvec,
                                    int n,
                                    int numEigen,
                                    int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    if (n <= 0 || numEigen <= 0) return {};

    numEigen = qMin(numEigen, n);
    int m = qMin(maxIter, n);   ///< 实际Lanczos迭代步数

    /* 初始向量: 随机单位向量 */
    QVector<double> v(n, 1.0 / qSqrt(static_cast<double>(n)));
    QVector<double> vPrev(n, 0.0);

    /* 三对角矩阵的对角和次对角 */
    QVector<double> alpha(m, 0.0);     ///< 对角元素
    QVector<double> beta(m + 1, 0.0);  ///< 次对角元素(beta[0]=0)

    /* Lanczos基向量 */
    QVector<QVector<double>> Q(m + 1);

    /* 初始归一化 */
    double beta0 = vecNorm(v);
    if (beta0 < 1e-15) {
        v[0] = 1.0;
        beta0 = 1.0;
    }
    for (int i = 0; i < n; ++i) v[i] /= beta0;
    Q[0] = v;
    beta[0] = 0.0;

    int actualIter = 0;
    for (int j = 0; j < m; ++j) {
        /* w = A * q_j */
        QVector<double> w = matvec(Q[j]);

        /* alpha_j = q_j^T * w */
        alpha[j] = dot(Q[j], w);

        /* w = w - alpha_j * q_j - beta_j * q_{j-1} */
        for (int i = 0; i < n; ++i) {
            w[i] -= alpha[j] * Q[j][i];
            if (j > 0) {
                w[i] -= beta[j] * Q[j - 1][i];
            }
        }

        /* 完全重正交化: 抵消浮点误差 */
        for (int reorth = 0; reorth < 2; ++reorth) {
            for (int k = 0; k <= j; ++k) {
                double proj = dot(w, Q[k]);
                for (int i = 0; i < n; ++i) {
                    w[i] -= proj * Q[k][i];
                }
            }
        }

        /* beta_{j+1} = ||w|| */
        beta[j + 1] = vecNorm(w);
        actualIter = j + 1;

        if (beta[j + 1] < 1e-15) {
            /* 不变子空间已找到 */
            break;
        }

        /* q_{j+1} = w / beta_{j+1} */
        Q[j + 1].resize(n);
        for (int i = 0; i < n; ++i) {
            Q[j + 1][i] = w[i] / beta[j + 1];
        }
    }

    /* 求解三对角矩阵的特征值 */
    QVector<double> eigenvalues = tridiagEigenvalues(alpha, beta, numEigen);

    /* 只返回有效结果 */
    if (eigenvalues.size() > numEigen) {
        eigenvalues.resize(numEigen);
    }

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalSolves;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalSolves);

    emit solveCompleted(eigenvalues.size(), actualIter);
    return eigenvalues;
}

/** @brief 向量点积 */
double LanczosEigen::dot(const QVector<double>& a,
                         const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/** @brief 向量范数 */
double LanczosEigen::vecNorm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/** @brief 三对角矩阵特征值求解(隐式QR Wilkinson位移) */
QVector<double> LanczosEigen::tridiagEigenvalues(
    const QVector<double>& alpha,
    const QVector<double>& beta,
    int numEigen)
{
    int n = alpha.size();
    if (n == 0) return {};

    /* 复制到工作数组 */
    QVector<double> d = alpha;               ///< 对角线
    QVector<double> e(n, 0.0);               ///< 次对角线
    for (int i = 0; i < n - 1; ++i) {
        e[i] = beta[i + 1];                  ///< e[i] = beta_{i+1}
    }

    /* 隐式QR迭代求解三对角特征值 */
    for (int l = 0; l < n; ++l) {
        int iter = 0;
        int mIdx = l;

        while (iter < 30) {
            /* 找到不可约块 [l, mIdx] */
            for (mIdx = l; mIdx < n - 1; ++mIdx) {
                double dd = qAbs(d[mIdx]) + qAbs(d[mIdx + 1]);
                if (qAbs(e[mIdx]) + dd == dd) {
                    e[mIdx] = 0.0;
                    break;
                }
            }

            if (mIdx == l) break;

            /* Wilkinson位移 */
            double g = (d[l + 1] - d[l]) / (2.0 * e[l]);
            double r = qSqrt(g * g + 1.0);
            double shift = d[mIdx] - d[l] + e[l] / (g + (g >= 0 ? r : -r));

            double f = 1.0;
            double c = 1.0;
            double s = 0.0;

            for (int i = l; i < mIdx; ++i) {
                double fi = f;
                double ci = c;
                f = d[i + 1] - shift;
                r = qSqrt(f * f + e[i] * e[i]);

                if (r < 1e-30) {
                    d[i] = ci * e[i];
                    e[i] = fi * s;
                    d[i + 1] = f;
                    break;
                }

                double p = f / r;
                double q2 = e[i] / r;
                f = ci * p + fi * q2;
                s = ci * q2 - fi * p;
                double h = p * d[i];
                d[i] = h;
                e[i] = s * d[i + 1];
                d[i + 1] += h;
                c = p;
            }

            ++iter;
        }
    }

    /* 排序(降序) */
    std::sort(d.begin(), d.end(), std::greater<double>());

    QVector<double> result(qMin(numEigen, n));
    for (int i = 0; i < result.size(); ++i) {
        result[i] = d[i];
    }
    return result;
}

/** @brief 重置统计 */
void LanczosEigen::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
