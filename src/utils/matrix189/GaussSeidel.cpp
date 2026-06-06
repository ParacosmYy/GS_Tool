/**
 * @file GaussSeidel.cpp
 * @brief GaussSeidel 实现
 *
 * 实现Gauss-Seidel迭代求解器：SOR加速、红黑排序、对角占优检测、收敛监测。
 */

#include "utils/matrix189/GaussSeidel.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussSeidel::GaussSeidel(QObject *parent) : QObject(parent) {}
GaussSeidel::~GaussSeidel() = default;

/* ---- Configuration ---- */

void GaussSeidel::setMaxIterations(int iter) { m_maxIterations = qMax(1, iter); }
void GaussSeidel::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }
void GaussSeidel::setOmega(double omega) { m_omega = qBound(0.0, omega, 2.0); }
void GaussSeidel::setRedBlackOrdering(bool enabled) { m_redBlack = enabled; }

/* ---- Diagonal dominance check ---- */

bool GaussSeidel::isDiagonallyDominant(const QVector<QVector<double>>& A) const
{
    int n = A.size();
    for (int i = 0; i < n; ++i) {
        double diag = qAbs(A[i][i]);
        double sum = 0.0;
        for (int j = 0; j < n; ++j) {
            if (j != i) sum += qAbs(A[i][j]);
        }
        if (diag < sum) return false;
    }
    return true;
}

/* ---- Residual computation ---- */

double GaussSeidel::residual(const QVector<QVector<double>>& A,
                              const QVector<double>& x,
                              const QVector<double>& b) const
{
    int n = A.size();
    double maxRes = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (int j = 0; j < n; ++j)
            ax += A[i][j] * x[j];
        maxRes = qMax(maxRes, qAbs(ax - b[i]));
    }
    return maxRes;
}

/* ---- Build color map for 2D grid ---- */

QVector<int> GaussSeidel::buildColorMap(int n) const
{
    // Assume n is a perfect square grid: sqrt(n) x sqrt(n)
    int side = qRound(qSqrt(static_cast<double>(n)));
    QVector<int> colors(n, 0);
    for (int i = 0; i < side; ++i)
        for (int j = 0; j < side; ++j)
            if (i * side + j < n)
                colors[i * side + j] = (i + j) % 2;
    return colors;
}

/* ---- Standard Gauss-Seidel with SOR ---- */

QVector<double> GaussSeidel::solveStandard(const QVector<QVector<double>>& A,
                                            const QVector<double>& b)
{
    int n = A.size();
    if (n == 0) return {};

    QVector<double> x(n, 0.0);

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        for (int i = 0; i < n; ++i) {
            double sigma = 0.0;
            for (int j = 0; j < n; ++j) {
                if (j != i) sigma += A[i][j] * x[j];
            }
            double xNew = (b[i] - sigma) / A[i][i];
            // SOR update
            x[i] = x[i] + m_omega * (xNew - x[i]);
        }

        double res = residual(A, x, b);
        if (res < m_tolerance) {
            m_stats.iterationsUsed = iter + 1;
            m_stats.finalResidual = res;
            return x;
        }
    }

    m_stats.iterationsUsed = m_maxIterations;
    m_stats.finalResidual = residual(A, x, b);
    return x;
}

/* ---- Red-black Gauss-Seidel ---- */

QVector<double> GaussSeidel::solveRedBlack(const QVector<QVector<double>>& A,
                                            const QVector<double>& b)
{
    int n = A.size();
    if (n == 0) return {};

    QVector<double> x(n, 0.0);
    auto colors = buildColorMap(n);

    for (int iter = 0; iter < m_maxIterations; ++iter) {
        // Update red nodes (color 0) first
        for (int color = 0; color < 2; ++color) {
            for (int i = 0; i < n; ++i) {
                if (colors[i] != color) continue;
                double sigma = 0.0;
                for (int j = 0; j < n; ++j) {
                    if (j != i) sigma += A[i][j] * x[j];
                }
                double xNew = (b[i] - sigma) / A[i][i];
                x[i] = x[i] + m_omega * (xNew - x[i]);
            }
        }

        double res = residual(A, x, b);
        if (res < m_tolerance) {
            m_stats.iterationsUsed = iter + 1;
            m_stats.finalResidual = res;
            return x;
        }
    }

    m_stats.iterationsUsed = m_maxIterations;
    m_stats.finalResidual = residual(A, x, b);
    return x;
}

/* ---- Main solve ---- */

QVector<double> GaussSeidel::solve(const QVector<QVector<double>>& A,
                                    const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    QVector<double> result;
    int n = A.size();
    if (n == 0 || b.size() != n) return result;

    // Check square matrix
    for (int i = 0; i < n; ++i)
        if (A[i].size() != n) return result;

    if (m_redBlack)
        result = solveRedBlack(A, b);
    else
        result = solveStandard(A, b);

    m_stats.totalSolves++;
    m_stats.matrixSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_stats.iterationsUsed, m_stats.finalResidual);
    return result;
}

/* ---- Reset ---- */

void GaussSeidel::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
