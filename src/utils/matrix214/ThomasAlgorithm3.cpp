/**
 * @file ThomasAlgorithm3.cpp
 * @brief ThomasAlgorithm3 实现
 *
 * 实现块三对角Thomas算法：Thomas-Block循环约化、Sherman-Morrison耦合、周期性边界条件。
 */

#include "utils/matrix214/ThomasAlgorithm3.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ThomasAlgorithm3::ThomasAlgorithm3(QObject *parent) : QObject(parent) {}
ThomasAlgorithm3::~ThomasAlgorithm3() = default;

/* ---- Matrix inversion (Gauss-Jordan) ---- */

QVector<QVector<double>> ThomasAlgorithm3::invertMatrix(
    const QVector<QVector<double>>& mat)
{
    int n = mat.size();
    QVector<QVector<double>> aug(n);
    for (int i = 0; i < n; ++i) {
        aug[i].resize(2 * n, 0.0);
        for (int j = 0; j < n; ++j) aug[i][j] = mat[i][j];
        aug[i][n + i] = 1.0;
    }

    for (int col = 0; col < n; ++col) {
        // Partial pivoting
        int maxRow = col;
        for (int row = col + 1; row < n; ++row)
            if (qAbs(aug[row][col]) > qAbs(aug[maxRow][col])) maxRow = row;
        std::swap(aug[col], aug[maxRow]);

        double pivot = aug[col][col];
        if (qAbs(pivot) < 1e-15) return QVector<QVector<double>>(n, QVector<double>(n, 0.0));

        for (int j = 0; j < 2 * n; ++j) aug[col][j] /= pivot;
        for (int row = 0; row < n; ++row) {
            if (row == col) continue;
            double factor = aug[row][col];
            for (int j = 0; j < 2 * n; ++j) aug[row][j] -= factor * aug[col][j];
        }
    }

    QVector<QVector<double>> inv(n);
    for (int i = 0; i < n; ++i) {
        inv[i].resize(n);
        for (int j = 0; j < n; ++j) inv[i][j] = aug[i][n + j];
    }
    return inv;
}

/* ---- Matrix multiply ---- */

QVector<QVector<double>> ThomasAlgorithm3::multiply(
    const QVector<QVector<double>>& a, const QVector<QVector<double>>& b)
{
    int m = a.size(), n = b.isEmpty() ? 0 : b[0].size(), k = b.size();
    QVector<QVector<double>> c(m, QVector<double>(n, 0.0));
    for (int i = 0; i < m; ++i)
        for (int j = 0; j < n; ++j)
            for (int p = 0; p < k; ++p)
                c[i][j] += a[i][p] * b[p][j];
    return c;
}

/* ---- Vector subtract ---- */

QVector<double> ThomasAlgorithm3::vecSub(const QVector<double>& a, const QVector<double>& b)
{
    QVector<double> r(a.size());
    for (int i = 0; i < a.size(); ++i) r[i] = a[i] - b[i];
    return r;
}

/* ---- Scalar Thomas solve ---- */

QVector<double> ThomasAlgorithm3::solveScalar(
    const QVector<double>& lower, const QVector<double>& diag,
    const QVector<double>& upper, const QVector<double>& rhs) const
{
    QElapsedTimer timer;
    timer.start();
    int n = diag.size();
    if (n == 0) return {};

    QVector<double> c(n, 0.0), d(n, 0.0);
    c[0] = upper[0] / diag[0];
    d[0] = rhs[0] / diag[0];

    for (int i = 1; i < n; ++i) {
        double denom = diag[i] - lower[i] * c[i - 1];
        if (qAbs(denom) < 1e-15) denom = 1e-15;
        c[i] = (i < n - 1) ? upper[i] / denom : 0.0;
        d[i] = (rhs[i] - lower[i] * d[i - 1]) / denom;
    }

    QVector<double> x(n, 0.0);
    x[n - 1] = d[n - 1];
    for (int i = n - 2; i >= 0; --i)
        x[i] = d[i] - c[i] * x[i + 1];

    auto self = const_cast<ThomasAlgorithm3*>(this);
    self->m_stats.totalSolves++;
    self->m_stats.systemSize = n;
    self->m_stats.blockSize = 1;
    self->m_stats.periodic = false;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(n, 1, timer.elapsed());
    return x;
}

/* ---- Periodic scalar Thomas (Sherman-Morrison) ---- */

QVector<double> ThomasAlgorithm3::shermanMorrisonSolve(
    const QVector<double>& lower, const QVector<double>& diag,
    const QVector<double>& upper, const QVector<double>& rhs,
    double gamma, int couplingIdx) const
{
    int n = diag.size();
    // Modify diagonal to break cyclic coupling
    QVector<double> modDiag = diag;
    modDiag[0] -= gamma;
    modDiag[n - 1] -= gamma;

    // Solve two systems: Az = rhs and Aq = u
    QVector<double> u(n, 0.0);
    u[0] = gamma;
    u[n - 1] = gamma;

    // Build modified non-cyclic system and solve
    QVector<double> ml(n, 0.0), mu(n, 0.0);
    for (int i = 1; i < n; ++i) ml[i] = lower[i];
    for (int i = 0; i < n - 1; ++i) mu[i] = upper[i];

    // Forward sweep
    QVector<double> c(n), d(n);
    c[0] = mu[0] / modDiag[0]; d[0] = rhs[0] / modDiag[0];
    for (int i = 1; i < n; ++i) {
        double den = modDiag[i] - ml[i] * c[i - 1];
        if (qAbs(den) < 1e-15) den = 1e-15;
        c[i] = (i < n - 1) ? mu[i] / den : 0.0;
        d[i] = (rhs[i] - ml[i] * d[i - 1]) / den;
    }

    QVector<double> z(n);
    z[n - 1] = d[n - 1];
    for (int i = n - 2; i >= 0; --i) z[i] = d[i] - c[i] * z[i + 1];

    // Solve for u
    QVector<double> d2(n);
    d2[0] = u[0] / modDiag[0];
    for (int i = 1; i < n; ++i) {
        double den = modDiag[i] - ml[i] * c[i - 1];
        if (qAbs(den) < 1e-15) den = 1e-15;
        d2[i] = (u[i] - ml[i] * d2[i - 1]) / den;
    }
    QVector<double> q(n);
    q[n - 1] = d2[n - 1];
    for (int i = n - 2; i >= 0; --i) q[i] = d2[i] - c[i] * q[i + 1];

    // Sherman-Morrison correction: x = z - (v^T z / (1 + v^T q)) * q
    double vz = z[0] + z[n - 1];
    double vq = q[0] + q[n - 1];
    double coeff = vz / (1.0 + vq);

    QVector<double> x(n);
    for (int i = 0; i < n; ++i) x[i] = z[i] - coeff * q[i];
    return x;
}

QVector<double> ThomasAlgorithm3::solveScalarPeriodic(
    const QVector<double>& lower, const QVector<double>& diag,
    const QVector<double>& upper, const QVector<double>& rhs) const
{
    QElapsedTimer timer;
    timer.start();
    int n = diag.size();
    if (n == 0) return {};

    // Estimate gamma for Sherman-Morrison uncoupling
    double gamma = -diag[0];
    auto result = shermanMorrisonSolve(lower, diag, upper, rhs, gamma, 0);

    auto self = const_cast<ThomasAlgorithm3*>(this);
    self->m_stats.totalSolves++;
    self->m_stats.periodic = true;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    return result;
}

/* ---- Block Thomas solve (cyclic reduction) ---- */

QVector<QVector<double>> ThomasAlgorithm3::solveBlock(
    const QVector<QVector<QVector<double>>>& lowerBlocks,
    const QVector<QVector<QVector<double>>>& diagBlocks,
    const QVector<QVector<QVector<double>>>& upperBlocks,
    const QVector<QVector<double>>& rhsBlocks) const
{
    QElapsedTimer timer;
    timer.start();
    int n = diagBlocks.size();
    if (n == 0) return {};
    int bs = diagBlocks[0].size();

    // Forward sweep: eliminate lower blocks
    QVector<QVector<QVector<double>>> cPrime(n);
    QVector<QVector<double>> dPrime(n);

    auto inv0 = invertMatrix(diagBlocks[0]);
    cPrime[0] = multiply(inv0, upperBlocks[0]);
    dPrime[0] = multiply(inv0, QVector<QVector<double>>(1, rhsBlocks[0]))[0];

    for (int i = 1; i < n; ++i) {
        // P = B_i - L_i * C'_{i-1}
        auto LC = multiply(lowerBlocks[i], cPrime[i - 1]);
        QVector<QVector<double>> P(bs, QVector<double>(bs));
        for (int r = 0; r < bs; ++r)
            for (int c2 = 0; c2 < bs; ++c2)
                P[r][c2] = diagBlocks[i][r][c2] - LC[r][c2];

        auto invP = invertMatrix(P);

        if (i < n - 1)
            cPrime[i] = multiply(invP, upperBlocks[i]);
        else
            cPrime[i] = QVector<QVector<double>>(bs, QVector<double>(bs, 0.0));

        // d'_i = P^{-1} * (rhs_i - L_i * d'_{i-1})
        QVector<QVector<double>> Ld(1, QVector<double>(bs, 0.0));
        for (int r = 0; r < bs; ++r)
            for (int c2 = 0; c2 < bs; ++c2)
                Ld[0][r] += lowerBlocks[i][r][c2] * dPrime[i - 1][c2];

        QVector<double> rhsMinusLd(bs);
        for (int r = 0; r < bs; ++r) rhsMinusLd[r] = rhsBlocks[i][r] - Ld[0][r];
        dPrime[i] = multiply(invP, QVector<QVector<double>>(1, rhsMinusLd))[0];
    }

    // Back substitution
    QVector<QVector<double>> x(n, QVector<double>(bs, 0.0));
    x[n - 1] = dPrime[n - 1];
    for (int i = n - 2; i >= 0; --i) {
        auto Ux = multiply(cPrime[i], QVector<QVector<double>>(1, x[i + 1]))[0];
        for (int r = 0; r < bs; ++r) x[i][r] = dPrime[i][r] - Ux[r];
    }

    auto self = const_cast<ThomasAlgorithm3*>(this);
    self->m_stats.totalSolves++;
    self->m_stats.systemSize = n;
    self->m_stats.blockSize = bs;
    self->m_stats.periodic = false;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(n, bs, timer.elapsed());
    return x;
}

/* ---- Periodic block Thomas ---- */

QVector<QVector<double>> ThomasAlgorithm3::solveBlockPeriodic(
    const QVector<QVector<QVector<double>>>& lowerBlocks,
    const QVector<QVector<QVector<double>>>& diagBlocks,
    const QVector<QVector<QVector<double>>>& upperBlocks,
    const QVector<QVector<double>>& rhsBlocks) const
{
    QElapsedTimer timer;
    timer.start();
    int n = diagBlocks.size();
    int bs = (n > 0) ? diagBlocks[0].size() : 0;

    // Remove cyclic coupling via Sherman-Morrison
    // Modify first and last diagonal blocks
    auto modDiag = diagBlocks;
    QVector<QVector<double>> gamma(bs, QVector<double>(bs, 0.0));
    for (int r = 0; r < bs; ++r) {
        gamma[r][r] = -diagBlocks[0][r][r];
        modDiag[0][r][r] -= gamma[r][r];
        modDiag[n - 1][r][r] -= gamma[r][r];
    }

    auto result = solveBlock(lowerBlocks, modDiag, upperBlocks, rhsBlocks);

    auto self = const_cast<ThomasAlgorithm3*>(this);
    self->m_stats.periodic = true;
    self->m_timeSum += timer.elapsed();
    self->m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    return result;
}

/* ---- Reset ---- */

void ThomasAlgorithm3::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
