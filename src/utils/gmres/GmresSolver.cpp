/**
 * @file GmresSolver.cpp
 * @brief GMRES求解器实现 — Arnoldi过程 + 最小二乘残差
 */

#include "utils/gmres/GmresSolver.h"

#include <QElapsedTimer>
#include <cmath>

GmresSolver::GmresSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 向量二范数 */
static double vecNorm(const QVector<double>& v)
{
    double s = 0.0;
    for (double x : v) s += x * x;
    return std::sqrt(s);
}

/** @brief 矩阵向量乘 */
static QVector<double> matVec(const QVector<QVector<double>>& A,
                              const QVector<double>& v)
{
    int n = A.size();
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double s = 0.0;
        for (int j = 0; j < n; ++j) {
            s += A[i][j] * v[j];
        }
        y[i] = s;
    }
    return y;
}

/** @brief 上三角回代 */
static QVector<double> backSub(const QVector<QVector<double>>& H,
                               const QVector<double>& rhs, int k)
{
    QVector<double> y(k, 0.0);
    for (int i = k - 1; i >= 0; --i) {
        y[i] = rhs[i];
        for (int j = i + 1; j < k; ++j) y[i] -= H[i][j] * y[j];
        if (std::abs(H[i][i]) > 1e-30) y[i] /= H[i][i];
    }
    return y;
}

/**
 * @brief Arnoldi单步: 计算 w=A*v_k, 正交化, 并应用Givens旋转
 * @return 实际执行的步数k
 */
static int arnoldiStep(const QVector<QVector<double>>& matA, int n,
                       QVector<QVector<double>>& V,
                       QVector<QVector<double>>& H,
                       QVector<double>& cs, QVector<double>& sn,
                       QVector<double>& beta, int m,
                       int maxIter, int& totalIter,
                       double bNorm, double tol, double& residual)
{
    int k = 0;
    for (k = 0; k < m; ++k) {
        /* w = A * V[k] */
        QVector<double> w = matVec(matA, V[k]);

        /* 修正Gram-Schmidt正交化 */
        for (int j = 0; j <= k; ++j) {
            double dot = 0.0;
            for (int i = 0; i < n; ++i) dot += w[i] * V[j][i];
            H[j][k] = dot;
            for (int i = 0; i < n; ++i) w[i] -= H[j][k] * V[j][i];
        }

        double wNorm = vecNorm(w);
        H[k + 1][k] = wNorm;
        if (wNorm < 1e-30) { ++k; break; }
        for (int i = 0; i < n; ++i) V[k + 1][i] = w[i] / wNorm;

        /* 应用之前的Givens旋转 */
        for (int j = 0; j < k; ++j) {
            double temp = cs[j] * H[j][k] + sn[j] * H[j + 1][k];
            H[j + 1][k] = -sn[j] * H[j][k] + cs[j] * H[j + 1][k];
            H[j][k] = temp;
        }

        /* 新Givens旋转消除H[k+1][k] */
        double rVal = std::sqrt(H[k][k] * H[k][k] + H[k + 1][k] * H[k + 1][k]);
        if (rVal > 1e-30) { cs[k] = H[k][k] / rVal; sn[k] = H[k + 1][k] / rVal; }
        else { cs[k] = 1.0; sn[k] = 0.0; }
        H[k][k] = cs[k] * H[k][k] + sn[k] * H[k + 1][k];
        H[k + 1][k] = 0.0;

        beta[k + 1] = -sn[k] * beta[k];
        beta[k] = cs[k] * beta[k];

        residual = std::abs(beta[k + 1]) / bNorm;
        ++totalIter;
        if (residual < tol) { ++k; break; }
        if (totalIter >= maxIter) { ++k; break; }
    }
    return k;
}

QVector<double> GmresSolver::solve(
    const QVector<QVector<double>>& matA,
    const QVector<double>& vecB,
    int restart, double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int n = matA.size();
    if (n == 0 || vecB.size() != n) {
        emit solveCompleted(0, 0.0);
        return {};
    }

    double bNorm = vecNorm(vecB);
    if (bNorm < 1e-30) {
        emit solveCompleted(0, 0.0);
        return QVector<double>(n, 0.0);
    }

    restart = qMin(qMax(restart, 1), n);
    QVector<double> x(n, 0.0);
    int totalIter = 0;
    double residual = 1.0;

    for (int outer = 0; outer < maxIter; ++outer) {
        /* 残差 r = b - A*x */
        QVector<double> ax = matVec(matA, x);
        QVector<double> r(n);
        for (int i = 0; i < n; ++i) r[i] = vecB[i] - ax[i];

        double rNorm = vecNorm(r);
        residual = rNorm / bNorm;
        if (residual < tol) break;

        /* 分配Arnoldi工作空间 */
        int m = restart;
        QVector<QVector<double>> V(m + 1, QVector<double>(n, 0.0));
        QVector<QVector<double>> H(m + 1, QVector<double>(m, 0.0));
        QVector<double> cs(m, 0.0), sn(m, 0.0), beta(m + 1, 0.0);

        for (int i = 0; i < n; ++i) V[0][i] = r[i] / rNorm;
        beta[0] = rNorm;

        int k = arnoldiStep(matA, n, V, H, cs, sn, beta, m,
                            maxIter, totalIter, bNorm, tol, residual);

        /* 回代更新解 */
        QVector<double> y = backSub(H, beta, k);
        for (int j = 0; j < k; ++j)
            for (int i = 0; i < n; ++i)
                x[i] += y[j] * V[j][i];

        if (residual < tol) break;
        if (totalIter >= maxIter) break;
    }

    ++m_stats.totalSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(totalIter, residual);
    return x;
}

void GmresSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
