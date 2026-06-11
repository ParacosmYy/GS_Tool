/**
 * @file ThomasAlgorithm8.cpp
 * @brief ThomasAlgorithm8 实现
 *
 * 实现追赶法：部分选主元与边界条件处理实现鲁棒三对角线性方程组求解。
 */

#include "utils/matrix298/ThomasAlgorithm8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

ThomasAlgorithm8::ThomasAlgorithm8(QObject *parent)
    : QObject(parent) {}

ThomasAlgorithm8::~ThomasAlgorithm8() = default;

/* ---- Configuration ---- */

void ThomasAlgorithm8::setBoundaryConditions(BoundaryType left, double leftValue,
                                               BoundaryType right, double rightValue)
{
    m_leftType = left;
    m_leftValue = leftValue;
    m_rightType = right;
    m_rightValue = rightValue;
}

/* ---- Apply boundary conditions ---- */

void ThomasAlgorithm8::applyBoundaryConditions(QVector<double>& mainDiag,
                                                 QVector<double>& upper,
                                                 QVector<double>& lower,
                                                 QVector<double>& rhs) const
{
    int n = mainDiag.size();
    if (n == 0) return;

    // Left boundary
    if (m_leftType == Dirichlet) {
        // x[0] = leftValue: modify first equation
        // Already handled: just ensure the system enforces x[0] = leftValue
        // Set diag[0] = 1, upper[0] = 0, rhs[0] = leftValue
        if (n > 1) {
            rhs[1] -= lower[1] * m_leftValue;
        }
        mainDiag[0] = 1.0;
        if (n > 0) upper[0] = 0.0;
        rhs[0] = m_leftValue;
    } else if (m_leftType == Neumann) {
        // dx/dx[0] = leftValue: use forward difference approximation
        // (x[1] - x[0]) / h = leftValue => modify first equation
        if (n > 1) {
            mainDiag[0] = -1.0;
            upper[0] = 1.0;
            rhs[0] = m_leftValue;
        }
    }

    // Right boundary
    if (m_rightType == Dirichlet) {
        if (n > 1) {
            rhs[n - 2] -= upper[n - 2] * m_rightValue;
        }
        mainDiag[n - 1] = 1.0;
        if (n > 1) lower[n - 1] = 0.0;
        rhs[n - 1] = m_rightValue;
    } else if (m_rightType == Neumann) {
        if (n > 1) {
            lower[n - 1] = 1.0;
            mainDiag[n - 1] = -1.0;
            rhs[n - 1] = m_rightValue;
        }
    }
}

/* ---- Standard Thomas algorithm (forward elimination + back substitution) ---- */

ThomasAlgorithm8::SolveResult ThomasAlgorithm8::solve(
    const QVector<double>& lower,
    const QVector<double>& mainDiag,
    const QVector<double>& upper,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = mainDiag.size();
    if (n == 0) {
        result.timeMs = timer.elapsed();
        return result;
    }

    result.systemSize = n;

    // Working copies
    QVector<double> a = lower;       // Sub-diagonal
    QVector<double> b = mainDiag;    // Main diagonal
    QVector<double> c = upper;       // Super-diagonal
    QVector<double> d = rhs;         // Right-hand side

    // Apply boundary conditions
    applyBoundaryConditions(b, c, a, d);

    // Forward elimination (forward sweep)
    QVector<double> cStar(n, 0.0);
    QVector<double> dStar(n, 0.0);

    cStar[0] = c[0] / b[0];
    dStar[0] = d[0] / b[0];

    for (int i = 1; i < n; ++i) {
        double m = b[i] - a[i] * cStar[i - 1];
        // Check for near-zero pivot
        if (qAbs(m) < 1e-15) {
            result.converged = false;
            m = 1e-15; // Regularize
        }
        if (i < n - 1)
            cStar[i] = c[i] / m;
        dStar[i] = (d[i] - a[i] * dStar[i - 1]) / m;
    }

    // Back substitution
    result.solution.resize(n);
    result.solution[n - 1] = dStar[n - 1];
    for (int i = n - 2; i >= 0; --i)
        result.solution[i] = dStar[i] - cStar[i] * result.solution[i + 1];

    // Compute residual
    result.residual = computeResidual(lower, mainDiag, upper, rhs, result.solution);

    double elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.maxSystemSize = qMax(m_stats.maxSystemSize, n);
    m_resSum += result.residual;
    m_stats.avgResidual = m_resSum / m_stats.totalSolves;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, result.residual, elapsed);
    return result;
}

/* ---- Solve with partial pivoting ---- */

ThomasAlgorithm8::SolveResult ThomasAlgorithm8::solveWithPivoting(
    const QVector<double>& lower,
    const QVector<double>& mainDiag,
    const QVector<double>& upper,
    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = mainDiag.size();
    if (n == 0) {
        result.timeMs = timer.elapsed();
        return result;
    }
    result.systemSize = n;

    // Build augmented matrix (tridiagonal stored as full for pivoting)
    // For efficiency, we only store the 3 diagonals + rhs but swap rows
    QVector<double> a = lower;
    QVector<double> b = mainDiag;
    QVector<double> c = upper;
    QVector<double> d = rhs;

    applyBoundaryConditions(b, c, a, d);

    // Forward elimination with partial pivoting
    // For tridiagonal: compare |b[i]| with |a[i]|, swap if needed
    QVector<double> cStar(n, 0.0);
    QVector<double> dStar(n, 0.0);

    // First row
    cStar[0] = c[0] / b[0];
    dStar[0] = d[0] / b[0];

    for (int i = 1; i < n; ++i) {
        // Partial pivoting: swap row i with row i-1 if |a[i]| > |b[i]|
        // This means the sub-diagonal element is larger than diagonal
        bool swapped = false;
        if (qAbs(a[i]) > qAbs(b[i]) && i > 0) {
            // Swap rows i-1 and i in the elimination context
            // Row i-1: b[i-1], cStar[i-1], dStar[i-1]
            // Row i: a[i], b[i], c[i], d[i]
            // After swap: row i gets modified previous row
            double newDiag = a[i];
            double newUpper = b[i];
            double newRhs = d[i];

            // The swapped row's relation changes
            double m = cStar[i - 1] / newDiag;
            b[i] = newUpper - m * a[i];
            if (qAbs(b[i]) < 1e-15) {
                result.converged = false;
                b[i] = 1e-15;
            }
            if (i < n - 1) cStar[i] = (c[i] - m * 0.0) / b[i]; // Simplified
            dStar[i] = (newRhs - m * dStar[i - 1]) / b[i];
            swapped = true;
        }

        if (!swapped) {
            double m = b[i] - a[i] * cStar[i - 1];
            if (qAbs(m) < 1e-15) {
                result.converged = false;
                m = 1e-15;
            }
            if (i < n - 1)
                cStar[i] = c[i] / m;
            dStar[i] = (d[i] - a[i] * dStar[i - 1]) / m;
        }
    }

    // Back substitution
    result.solution.resize(n);
    result.solution[n - 1] = dStar[n - 1];
    for (int i = n - 2; i >= 0; --i)
        result.solution[i] = dStar[i] - cStar[i] * result.solution[i + 1];

    result.residual = computeResidual(lower, mainDiag, upper, rhs, result.solution);

    double elapsed = timer.elapsed();
    m_stats.totalSolves++;
    m_stats.maxSystemSize = qMax(m_stats.maxSystemSize, n);
    m_resSum += result.residual;
    m_stats.avgResidual = m_resSum / m_stats.totalSolves;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, result.residual, elapsed);
    return result;
}

/* ---- Solve periodic system via Sherman-Morrison ---- */

ThomasAlgorithm8::SolveResult ThomasAlgorithm8::solvePeriodic(
    QVector<double> lower, QVector<double> mainDiag,
    QVector<double> upper, QVector<double> rhs)
{
    int n = mainDiag.size();
    SolveResult result;
    result.systemSize = n;
    if (n < 3) { result.converged = false; return result; }

    // Sherman-Morrison: decompose periodic system into two standard systems
    // Modify corners: mainDiag[0] -= gamma, mainDiag[n-1] -= alpha*beta/gamma
    double alpha = lower[0];
    double beta = upper[n - 1];
    double gamma = mainDiag[0];

    mainDiag[0] -= gamma;
    mainDiag[n - 1] -= alpha * beta / gamma;

    // Solve modified system
    QVector<double> u(n, 0.0);
    u[0] = gamma;
    u[n - 1] = alpha;

    auto solY = solve(lower, mainDiag, upper, rhs);
    auto solZ = solve(lower, mainDiag, upper, u);

    // Sherman-Morrison correction
    double factor = (solY.solution[0] + solY.solution[n - 1] * beta / gamma) /
                    (1.0 + solZ.solution[0] + solZ.solution[n - 1] * beta / gamma);

    result.solution.resize(n);
    for (int i = 0; i < n; ++i)
        result.solution[i] = solY.solution[i] - factor * solZ.solution[i];

    result.converged = solY.converged && solZ.converged;
    return result;
}

/* ---- Compute residual ---- */

double ThomasAlgorithm8::computeResidual(const QVector<double>& lower,
                                           const QVector<double>& mainDiag,
                                           const QVector<double>& upper,
                                           const QVector<double>& rhs,
                                           const QVector<double>& x) const
{
    int n = mainDiag.size();
    double maxRes = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = mainDiag[i] * x[i];
        if (i > 0) ax += lower[i] * x[i - 1];
        if (i < n - 1) ax += upper[i] * x[i + 1];
        maxRes = qMax(maxRes, qAbs(ax - rhs[i]));
    }
    return maxRes;
}

/* ---- Reset ---- */

void ThomasAlgorithm8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_resSum = 0.0;
}
