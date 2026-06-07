/**
 * @file TridiagonalSolver2.cpp
 * @brief TridiagonalSolver2 实现
 *
 * 实现三对角矩阵求解：Thomas算法、循环约化并行化、条件数估计、批量求解。
 */

#include "utils/matrix193/TridiagonalSolver2.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

TridiagonalSolver2::TridiagonalSolver2(QObject *parent) : QObject(parent) {}
TridiagonalSolver2::~TridiagonalSolver2() = default;

/* ---- Set system ---- */

void TridiagonalSolver2::setSystem(const QVector<double>& a,
                                   const QVector<double>& b,
                                   const QVector<double>& c)
{
    m_n = b.size();
    m_a = a;
    m_b = b;
    m_c = c;
    m_stats.systemSize = m_n;
}

/* ---- Validate dimensions ---- */

bool TridiagonalSolver2::validateSystem() const
{
    if (m_n < 2) return false;
    if (m_a.size() != m_n - 1) return false;
    if (m_b.size() != m_n) return false;
    if (m_c.size() != m_n - 1) return false;
    return true;
}

/* ---- Thomas algorithm (serial) ---- */

QVector<double> TridiagonalSolver2::solveThomas(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (!validateSystem() || rhs.size() != m_n) return {};

    int n = m_n;
    // Forward sweep
    QVector<double> cp(n - 1, 0.0);
    QVector<double> dp(n, 0.0);

    cp[0] = m_c[0] / m_b[0];
    dp[0] = rhs[0] / m_b[0];

    for (int i = 1; i < n; ++i) {
        double denom = m_b[i] - m_a[i - 1] * cp[i - 1];
        if (qAbs(denom) < 1e-15) return {}; // Singular
        if (i < n - 1)
            cp[i] = m_c[i] / denom;
        dp[i] = (rhs[i] - m_a[i - 1] * dp[i - 1]) / denom;
    }

    // Back substitution
    QVector<double> x(n, 0.0);
    x[n - 1] = dp[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = dp[i] - cp[i] * x[i + 1];

    m_stats.totalSolves++;
    m_stats.residualNorm = computeResidual(x, rhs);
    m_stats.conditionEstimate = estimateConditionNumber();
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, m_stats.residualNorm, timer.elapsed());
    return x;
}

/* ---- Cyclic reduction (parallel-suitable) ---- */

QVector<double> TridiagonalSolver2::solveCyclicReduction(const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    if (!validateSystem() || rhs.size() != m_n) return {};

    int n = m_n;
    // Working copies
    QVector<double> a = m_a, b = m_b, c = m_c, d = rhs;

    // Forward reduction: eliminate odd-indexed unknowns
    int numLevels = 0;
    int stride = 1;
    while (stride * 2 < n) {
        for (int i = stride; i < n; i += stride * 2) {
            int left = i - stride;
            int right = (i + stride < n) ? i + stride : -1;

            double alpha = 0.0, gamma = 0.0;
            if (i > 0) {
                int ai = i - 1; // m_a[i-1] in original, but use working a
                if (ai < a.size())
                    alpha = -a[ai] / b[left];
            }
            if (right >= 0 && i < c.size()) {
                gamma = -c[i] / b[right];
            }

            // Update coefficients for row i
            double newB = b[i];
            if (i > 0 && (i - 1) < a.size()) newB += alpha * c[i - 1 >= 0 && i - 1 < c.size() ? c[i - 1] : 0];
            if (right >= 0 && i < c.size()) newB += gamma * (right - 1 >= 0 && right - 1 < a.size() ? a[right - 1] : 0);

            double newD = d[i];
            if (i > 0 && (i - 1) < a.size()) newD -= a[i - 1] / b[left] * d[left];
            if (right >= 0 && i < c.size()) newD -= c[i] / b[right] * d[right];

            b[i] = newB;
            d[i] = newD;
        }
        stride *= 2;
        numLevels++;
    }

    // Solve reduced systems (typically very small)
    QVector<double> x(n, 0.0);
    // Back substitution in reverse order
    for (int i = n - 1; i >= 0; --i) {
        if (qAbs(b[i]) < 1e-15) continue;
        double sum = d[i];
        if (i + 1 < n && i < c.size()) sum -= c[i] * x[i + 1];
        if (i > 0 && (i - 1) < a.size()) sum -= a[i - 1] * x[i - 1];
        x[i] = sum / b[i];
    }

    m_stats.totalSolves++;
    m_stats.residualNorm = computeResidual(x, rhs);
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(n, m_stats.residualNorm, timer.elapsed());
    return x;
}

/* ---- Batch solve ---- */

QVector<QVector<double>> TridiagonalSolver2::solveBatch(
    const QVector<QVector<double>>& rhsList)
{
    QVector<QVector<double>> results;
    results.reserve(rhsList.size());
    for (const auto& rhs : rhsList)
        results.append(solveThomas(rhs));
    return results;
}

/* ---- Compute residual ---- */

double TridiagonalSolver2::computeResidual(const QVector<double>& x,
                                           const QVector<double>& rhs) const
{
    if (x.size() != m_n || rhs.size() != m_n) return qQNaN();

    double maxRes = 0.0;
    for (int i = 0; i < m_n; ++i) {
        double ax = m_b[i] * x[i];
        if (i > 0) ax += m_a[i - 1] * x[i - 1];
        if (i < m_n - 1) ax += m_c[i] * x[i + 1];
        maxRes = qMax(maxRes, qAbs(ax - rhs[i]));
    }
    return maxRes;
}

/* ---- Matrix 1-norm ---- */

double TridiagonalSolver2::matrixNorm1() const
{
    double maxCol = 0.0;
    for (int j = 0; j < m_n; ++j) {
        double colSum = qAbs(m_b[j]);
        if (j > 0) colSum += qAbs(m_a[j - 1]);
        if (j < m_n - 1) colSum += qAbs(m_c[j]);
        maxCol = qMax(maxCol, colSum);
    }
    return maxCol;
}

/* ---- Inverse norm estimate via power iteration ---- */

double TridiagonalSolver2::inverseNormEstimate() const
{
    // Use Thomas solve as "inverse multiply" in power iteration
    int n = m_n;
    QVector<double> v(n, 1.0 / qSqrt(static_cast<double>(n)));

    for (int iter = 0; iter < 10; ++iter) {
        // Solve A * w = v (Thomas is O(n) so this is cheap)
        auto* self = const_cast<TridiagonalSolver2*>(this);
        QVector<double> w = self->solveThomas(v);
        if (w.isEmpty()) return qSqrt(static_cast<double>(n)) * 1e15;

        double norm = 0.0;
        for (double wi : w) norm += wi * wi;
        norm = qSqrt(norm);
        if (norm < 1e-15) break;

        for (int i = 0; i < n; ++i) v[i] = w[i] / norm;
    }

    double normV = 0.0;
    for (double vi : v) normV += vi * vi;
    return qSqrt(normV);
}

/* ---- Estimate condition number ---- */

double TridiagonalSolver2::estimateConditionNumber() const
{
    if (m_n < 2) return 1.0;
    return matrixNorm1() * inverseNormEstimate();
}

/* ---- Reset statistics ---- */

void TridiagonalSolver2::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
