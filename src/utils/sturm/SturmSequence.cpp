/**
 * @file SturmSequence.cpp
 * @brief Sturm序列实现 — 特征值计数与隔离
 */

#include "utils/sturm/SturmSequence.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
SturmSequence::SturmSequence(QObject* parent)
    : QObject(parent)
{
}

/** @brief 计算小于x的特征值个数(符号变化数) */
int SturmSequence::countEigenvalues(const QVector<double>& diag,
                                     const QVector<double>& offDiag,
                                     double x) const
{
    int n = diag.size();
    if (n == 0) return 0;

    /* Sturm序列: p_0 = 1, p_1 = d_0 - x */
    double qPrev = 1.0;
    double qCurr = diag[0] - x;
    int count = (qCurr < 0.0) ? 1 : 0;

    for (int k = 1; k < n; ++k) {
        double qNext = (diag[k] - x) - offDiag[k - 1] * offDiag[k - 1] / qPrev;
        if (std::abs(qPrev) < 1e-300)
            qNext = (diag[k] - x) - 1e15;

        if (qNext * qCurr < 0.0) count++;

        qPrev = qCurr;
        qCurr = qNext;
    }

    return count;
}

/** @brief 隔离全部特征值(二分搜索) */
QVector<double> SturmSequence::isolateEigenvalues(
    const QVector<double>& diag,
    const QVector<double>& offDiag,
    double lower, double upper,
    int nEigenvalues)
{
    QElapsedTimer timer;
    timer.start();

    int n = diag.size();
    if (n == 0) return {};

    /* Gershgorin界 */
    double gLower = lower, gUpper = upper;
    if (gLower >= gUpper) {
        gLower = diag[0] - std::abs(offDiag.isEmpty() ? 0.0 : offDiag[0]);
        gUpper = diag[0] + std::abs(offDiag.isEmpty() ? 0.0 : offDiag[0]);
        for (int i = 1; i < n; ++i) {
            double off = (i < offDiag.size()) ? offDiag[i] : 0.0;
            double offPrev = offDiag[i - 1];
            double lo = diag[i] - std::abs(off) - std::abs(offPrev);
            double hi = diag[i] + std::abs(off) + std::abs(offPrev);
            if (lo < gLower) gLower = lo;
            if (hi > gUpper) gUpper = hi;
        }
    }

    QVector<double> eigenvalues;
    double tol = 1e-12;
    int maxIter = 100;

    /* 逐个隔离特征值 */
    int found = countEigenvalues(diag, offDiag, gUpper);
    int targetCount = (nEigenvalues > 0) ? nEigenvalues : n;

    for (int k = 0; k < targetCount && k < found; ++k) {
        double lo = gLower, hi = gUpper;
        int countLo = countEigenvalues(diag, offDiag, lo);

        for (int iter = 0; iter < maxIter; ++iter) {
            double mid = (lo + hi) / 2.0;
            int countMid = countEigenvalues(diag, offDiag, mid);

            if (countMid <= countLo + k)
                lo = mid;
            else
                hi = mid;

            if (hi - lo < tol) break;
        }

        eigenvalues.append((lo + hi) / 2.0);
    }

    m_stats.totalComputations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalComputations;

    emit computationCompleted(eigenvalues.size());
    return eigenvalues;
}

/** @brief 重置统计 */
void SturmSequence::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
