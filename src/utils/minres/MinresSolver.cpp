/**
 * @file MinresSolver.cpp
 * @brief MINRES求解器实现 — Lanczos三对角化 + 最小残差
 */

#include "utils/minres/MinresSolver.h"

#include <QElapsedTimer>
#include <cmath>

MinresSolver::MinresSolver(QObject* parent)
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
        for (int j = 0; j < n; ++j) s += A[i][j] * v[j];
        y[i] = s;
    }
    return y;
}

/**
 * @brief Lanczos单步: 计算A*v, 正交化, 返回三对角元素
 * @param vCurr 当前Lanczos向量
 * @param vPrev 前一个Lanczos向量
 * @param betaPrev 前一次对角线元素
 * @param[out] alphaCurr 当前对角线元素
 * @param[out] betaCurr  次对角线元素
 * @return 新的Lanczos向量
 */
static QVector<double> lanczosStep(
    const QVector<QVector<double>>& A, int n,
    const QVector<double>& vCurr, const QVector<double>& vPrev,
    double betaPrev, double& alphaCurr, double& betaCurr)
{
    QVector<double> w = matVec(A, vCurr);
    for (int i = 0; i < n; ++i) w[i] -= betaCurr * vPrev[i];

    alphaCurr = 0.0;
    for (int i = 0; i < n; ++i) alphaCurr += w[i] * vCurr[i];

    for (int i = 0; i < n; ++i) w[i] -= alphaCurr * vCurr[i];

    /* 一次重正交化 */
    double dot = 0.0;
    for (int i = 0; i < n; ++i) dot += w[i] * vCurr[i];
    for (int i = 0; i < n; ++i) w[i] -= dot * vCurr[i];

    betaCurr = vecNorm(w);
    if (betaCurr > 1e-30) {
        QVector<double> vNew(n);
        for (int i = 0; i < n; ++i) vNew[i] = w[i] / betaCurr;
        return vNew;
    }
    return vCurr;
}

QVector<double> MinresSolver::solve(
    const QVector<QVector<double>>& matA,
    const QVector<double>& vecB,
    double tol, int maxIter)
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

    QVector<double> x(n, 0.0);
    QVector<double> vPrev(n, 0.0), vCurr(n);
    for (int i = 0; i < n; ++i) vCurr[i] = vecB[i] / bNorm;

    double betaPrev = 0.0, betaCurr = 0.0, alphaCurr = 0.0;
    double eta = bNorm, oldc = 1.0, olds = 0.0;
    double gammaPrev = 0.0, gammaCurr = 0.0;
    double delta = 0.0, epsilon = 0.0;
    QVector<double> wCurr(n, 0.0), wPrev(n, 0.0);

    double residual = 1.0;
    int iter = 0;

    for (iter = 0; iter < maxIter; ++iter) {
        /* Lanczos步 */
        QVector<double> vNew = lanczosStep(matA, n, vCurr, vPrev,
                                           betaPrev, alphaCurr, betaCurr);
        betaPrev = betaCurr;

        /* Givens旋转更新 */
        double deltaOld = delta;
        delta = olds * betaPrev + oldc * alphaCurr;
        double diag2 = oldc * betaPrev - olds * alphaCurr;

        double r = std::sqrt(diag2 * diag2 + betaCurr * betaCurr);
        double cNew = 1.0, sNew = 0.0;
        if (r > 1e-30) { cNew = diag2 / r; sNew = betaCurr / r; }

        gammaPrev = gammaCurr;
        gammaCurr = r;
        epsilon = sNew * eta;
        eta = -cNew * eta;

        /* 更新搜索方向和解 */
        if (iter == 0) {
            for (int i = 0; i < n; ++i) wCurr[i] = vCurr[i] / gammaCurr;
        } else {
            for (int i = 0; i < n; ++i)
                wCurr[i] = (vCurr[i] - deltaOld * wPrev[i]) / gammaCurr;
        }
        double zeta = oldc * eta + olds * epsilon;
        for (int i = 0; i < n; ++i) x[i] += zeta * wCurr[i];

        residual = std::abs(eta) / bNorm;
        if (residual < tol) { ++iter; break; }

        oldc = cNew; olds = sNew;
        wPrev = wCurr;
        vPrev = vCurr;
        vCurr = vNew;
    }

    ++m_stats.totalSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(iter, residual);
    return x;
}

void MinresSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
