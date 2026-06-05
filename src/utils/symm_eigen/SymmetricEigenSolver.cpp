/**
 * @file SymmetricEigenSolver.cpp
 * @brief 对称矩阵特征值求解器实现 — 经典Jacobi旋转迭代
 */

#include "utils/symm_eigen/SymmetricEigenSolver.h"

#include <QElapsedTimer>
#include <cmath>

SymmetricEigenSolver::SymmetricEigenSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 寻找绝对值最大的非对角元素(p, q)
 * @return {maxVal, p, q}
 */
static QPair<double, QPair<int, int>>
    findMaxOffDiag(const QVector<QVector<double>>& S, int n)
{
    int p = 0, q = 1;
    double maxVal = 0.0;
    for (int i = 0; i < n; ++i) {
        for (int j = i + 1; j < n; ++j) {
            double aij = std::abs(S[i][j]);
            if (aij > maxVal) { maxVal = aij; p = i; q = j; }
        }
    }
    return {maxVal, {p, q}};
}

/**
 * @brief 对S和V执行一次Jacobi旋转
 */
static void jacobiRotate(QVector<QVector<double>>& S,
                         QVector<QVector<double>>& V,
                         int p, int q, int n)
{
    double app = S[p][p], aqq = S[q][q], apq = S[p][q];

    /* 计算旋转角 */
    double theta = 0.0;
    double diff = aqq - app;
    if (std::abs(diff) < 1e-30) {
        theta = (apq > 0) ? (M_PI / 4.0) : (-M_PI / 4.0);
    } else {
        theta = 0.5 * std::atan2(2.0 * apq, diff);
    }
    double c = std::cos(theta), s = std::sin(theta);

    /* S' = G^T * S * G: 更新行列 */
    for (int j = 0; j < n; ++j) {
        double spj = S[p][j], sqj = S[q][j];
        S[p][j] =  c * spj + s * sqj;
        S[q][j] = -s * spj + c * sqj;
    }
    for (int i = 0; i < n; ++i) {
        double sip = S[i][p], siq = S[i][q];
        S[i][p] =  c * sip + s * siq;
        S[i][q] = -s * sip + c * siq;
    }

    /* 精确更新对角和交叉 */
    S[p][p] = c * c * app + 2.0 * s * c * apq + s * s * aqq;
    S[q][q] = s * s * app - 2.0 * s * c * apq + c * c * aqq;
    S[p][q] = 0.0; S[q][p] = 0.0;

    /* V = V * G */
    for (int i = 0; i < n; ++i) {
        double vip = V[i][p], viq = V[i][q];
        V[i][p] =  c * vip + s * viq;
        V[i][q] = -s * vip + c * viq;
    }
}

QPair<QVector<double>, QVector<QVector<double>>>
SymmetricEigenSolver::solve(const QVector<QVector<double>>& matA,
                            int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int n = matA.size();
    if (n == 0) { emit solveCompleted(0, 0); return {{}, {}}; }

    QVector<QVector<double>> S = matA;
    QVector<QVector<double>> V(n, QVector<double>(n, 0.0));
    for (int i = 0; i < n; ++i) V[i][i] = 1.0;

    int iter = 0;
    for (iter = 0; iter < maxIter; ++iter) {
        auto [maxVal, pq] = findMaxOffDiag(S, n);
        if (maxVal < 1e-14) break;
        jacobiRotate(S, V, pq.first, pq.second, n);
    }

    QVector<double> eigenvalues(n);
    for (int i = 0; i < n; ++i) eigenvalues[i] = S[i][i];

    ++m_stats.totalSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, iter);
    return {eigenvalues, V};
}

void SymmetricEigenSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
