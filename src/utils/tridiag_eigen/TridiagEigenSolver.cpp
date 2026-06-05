/**
 * @file TridiagEigenSolver.cpp
 * @brief 三对角对称矩阵特征值求解器实现 — 隐式QR位移迭代
 */

#include "utils/tridiag_eigen/TridiagEigenSolver.h"

#include <QElapsedTimer>
#include <cmath>

TridiagEigenSolver::TridiagEigenSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/**
 * @brief 计算Wilkinson位移值
 */
static double wilkinsonShift(double dm1, double dm, double em1)
{
    double dd = (dm1 - dm) * 0.5;
    double mu = dm;
    if (std::abs(dd) > 1e-30) {
        double signDd = (dd >= 0) ? 1.0 : -1.0;
        mu -= em1 * em1 / (dd + signDd * std::sqrt(dd * dd + em1 * em1));
    }
    return mu;
}

/**
 * @brief 隐式QR步: Givens旋转链更新三对角元素 d[l..m], e[l..m-1]
 */
static void implicitQrStep(QVector<double>& d, QVector<double>& e,
                           int lIdx, int mIdx)
{
    double mu = wilkinsonShift(d[mIdx - 1], d[mIdx], e[mIdx - 1]);
    double x = d[lIdx] - mu;
    double z = e[lIdx];

    for (int k = lIdx; k < mIdx; ++k) {
        double r = std::sqrt(x * x + z * z);
        double c = 1.0, s = 0.0;
        if (r > 1e-30) { c = x / r; s = z / r; }

        if (k > lIdx) e[k - 1] = r;

        double dk = d[k], ek = e[k], dk1 = d[k + 1];
        d[k]     = c * c * dk + 2.0 * c * s * ek + s * s * dk1;
        d[k + 1] = s * s * dk - 2.0 * c * s * ek + c * c * dk1;
        e[k]     = c * s * (dk1 - dk) + (c * c - s * s) * ek;

        if (k + 1 < mIdx) {
            x = e[k]; z = s * e[k + 1]; e[k + 1] = c * e[k + 1];
        }
    }
}

QVector<double> TridiagEigenSolver::solve(
    const QVector<double>& diag,
    const QVector<double>& offDiag,
    int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    int n = diag.size();
    if (n == 0) { emit solveCompleted(0, 0); return {}; }

    QVector<double> d = diag;
    QVector<double> e(n, 0.0);
    if (offDiag.size() >= n - 1) {
        for (int i = 0; i < n - 1; ++i) e[i] = offDiag[i];
    }

    int totalIter = 0;
    int mIdx = n - 1;

    while (mIdx > 0) {
        /* 寻找底部非零次对角元素 */
        while (mIdx > 0 && std::abs(e[mIdx - 1])
               <= 1e-14 * (std::abs(d[mIdx - 1]) + std::abs(d[mIdx]))) {
            e[mIdx - 1] = 0.0; --mIdx;
        }
        if (mIdx == 0) break;

        /* 找到未收敛的起始点 */
        int lIdx = mIdx - 1;
        while (lIdx > 0 && std::abs(e[lIdx - 1])
               > 1e-14 * (std::abs(d[lIdx - 1]) + std::abs(d[lIdx]))) {
            --lIdx;
        }

        implicitQrStep(d, e, lIdx, mIdx);

        ++totalIter;
        if (totalIter >= maxIter) break;
    }

    std::sort(d.begin(), d.end());

    ++m_stats.totalSolves;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, totalIter);
    return d;
}

void TridiagEigenSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
