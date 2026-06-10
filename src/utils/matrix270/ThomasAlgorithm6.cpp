/**
 * @file ThomasAlgorithm6.cpp
 * @brief ThomasAlgorithm6 实现
 *
 * 实现Thomas算法：部分主元消去与循环约化扩展鲁棒三对角系统求解。
 */

#include "utils/matrix270/ThomasAlgorithm6.h"

#include <QElapsedTimer>
#include <QtGlobal>

/* ---- Construction / Destruction ---- */

ThomasAlgorithm6::ThomasAlgorithm6(QObject *parent)
    : QObject(parent) {}

ThomasAlgorithm6::~ThomasAlgorithm6() = default;

/* ---- Standard Thomas solve ---- */

QVector<double> ThomasAlgorithm6::thomasSolve(QVector<double> a, QVector<double> b,
                                                QVector<double> c, QVector<double> d)
{
    int n = b.size();
    if (n == 0) return {};

    // Forward elimination
    for (int i = 1; i < n; ++i) {
        double m = a[i] / b[i - 1];
        b[i] -= m * c[i - 1];
        d[i] -= m * d[i - 1];
        a[i] = 0.0;
    }

    // Back substitution
    QVector<double> x(n);
    x[n - 1] = d[n - 1] / b[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = (d[i] - c[i] * x[i + 1]) / b[i];

    return x;
}

/* ---- Standard solve ---- */

QVector<double> ThomasAlgorithm6::solve(const QVector<double>& lower,
                                          const QVector<double>& mainDiag,
                                          const QVector<double>& upper,
                                          const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    auto result = thomasSolve(lower, mainDiag, upper, rhs);

    double elapsed = timer.elapsed();
    m_stats.systemSize = mainDiag.size();
    m_stats.pivoted = false;
    m_stats.pivotCount = 0;
    m_stats.isCyclic = false;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit systemSolved(mainDiag.size(), false, 0, elapsed);
    return result;
}

/* ---- Solve with partial pivoting ---- */

QVector<double> ThomasAlgorithm6::solveWithPivoting(QVector<double> lower,
                                                      QVector<double> mainDiag,
                                                      QVector<double> upper,
                                                      QVector<double> rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = mainDiag.size();
    if (n == 0) return {};
    int pivotCount = 0;

    // Forward elimination with partial pivoting
    for (int i = 0; i < n - 1; ++i) {
        // Check if row swap needed: |lower[i+1]| > |mainDiag[i]|
        if (qAbs(lower[i + 1]) > qAbs(mainDiag[i])) {
            // Swap rows i and i+1
            std::swap(mainDiag[i], lower[i + 1]);
            std::swap(upper[i], mainDiag[i + 1]);
            // upper[i+1] stays in place (was sub-diagonal, now becomes 0)
            std::swap(rhs[i], rhs[i + 1]);
            pivotCount++;
        }

        // Standard elimination
        if (qAbs(mainDiag[i]) < 1e-15) continue;
        double m = lower[i + 1] / mainDiag[i];
        mainDiag[i + 1] -= m * upper[i];
        rhs[i + 1] -= m * rhs[i];
        lower[i + 1] = 0.0;
    }

    // Back substitution
    QVector<double> x(n);
    if (qAbs(mainDiag[n - 1]) > 1e-15)
        x[n - 1] = rhs[n - 1] / mainDiag[n - 1];
    else
        x[n - 1] = 0.0;

    for (int i = n - 2; i >= 0; --i) {
        if (qAbs(mainDiag[i]) > 1e-15)
            x[i] = (rhs[i] - upper[i] * x[i + 1]) / mainDiag[i];
        else
            x[i] = 0.0;
    }

    double elapsed = timer.elapsed();
    m_stats.systemSize = n;
    m_stats.pivoted = true;
    m_stats.pivotCount = pivotCount;
    m_stats.isCyclic = false;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit systemSolved(n, true, pivotCount, elapsed);
    return x;
}

/* ---- Solve cyclic tridiagonal via Sherman-Morrison ---- */

QVector<double> ThomasAlgorithm6::solveCyclic(
    const QVector<double>& lower, const QVector<double>& mainDiag,
    const QVector<double>& upper, const QVector<double>& rhs,
    double cornerLower, double cornerUpper)
{
    QElapsedTimer timer;
    timer.start();

    int n = mainDiag.size();
    if (n < 3) return {};

    // Sherman-Morrison: decompose cyclic system into standard + rank-1 correction
    // Modify main diagonal at corners
    QVector<double> bMod = mainDiag;
    double gamma = -bMod[0];  // Choose gamma = -b[0]
    bMod[0] -= gamma;
    bMod[n - 1] -= cornerLower * cornerUpper / gamma;

    // Solve modified system for two RHS vectors
    QVector<double> d1 = rhs;
    QVector<double> d2(n, 0.0);
    d2[0] = gamma;
    d2[n - 1] = cornerUpper;

    QVector<double> u = thomasSolve(lower, bMod, upper, d2);
    QVector<double> y = thomasSolve(lower, bMod, upper, d1);

    // Sherman-Morrison correction: x = y - ((v^T y)/(1 + v^T u)) * u
    // where v encodes the rank-1 modification
    double vDotY = y[0];    // v[0] = gamma / gamma = 1
    double vDotU = u[0];
    // v[n-1] = cornerLower / gamma
    vDotY += (cornerLower / gamma) * y[n - 1];
    vDotU += (cornerLower / gamma) * u[n - 1];

    double factor = vDotY / (1.0 + vDotU);

    QVector<double> x(n);
    for (int i = 0; i < n; ++i)
        x[i] = y[i] - factor * u[i];

    double elapsed = timer.elapsed();
    m_stats.systemSize = n;
    m_stats.pivoted = false;
    m_stats.isCyclic = true;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit systemSolved(n, false, 0, elapsed);
    return x;
}

/* ---- Reset ---- */

void ThomasAlgorithm6::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
