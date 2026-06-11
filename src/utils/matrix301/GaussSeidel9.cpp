/**
 * @file GaussSeidel9.cpp
 * @brief GaussSeidel9 实现
 *
 * 实现高斯-赛德尔迭代：红黑排序与逐次超松弛加速椭圆型PDE离散化收敛。
 */

#include "utils/matrix301/GaussSeidel9.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

GaussSeidel9::GaussSeidel9(QObject *parent)
    : QObject(parent) {}

GaussSeidel9::~GaussSeidel9() = default;

/* ---- Configuration ---- */

void GaussSeidel9::setMaxIterations(int maxIter) { m_maxIterations = qBound(1, maxIter, 1000000); }
void GaussSeidel9::setTolerance(double tol) { m_tolerance = qBound(1e-15, tol, 1.0); }
void GaussSeidel9::setRelaxationFactor(double omega) { m_omega = qBound(0.1, omega, 2.0); }

/* ---- Compute residual ||Ax - b||_inf ---- */

double GaussSeidel9::computeResidual(const QVector<QVector<double>>& A,
                                      const QVector<double>& x,
                                      const QVector<double>& b) const
{
    int n = x.size();
    double maxRes = 0.0;
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = 0; j < n; ++j)
            sum += A[i][j] * x[j];
        maxRes = qMax(maxRes, qAbs(sum - b[i]));
    }
    return maxRes;
}

/* ---- Red-black sweep for general sparse matrix (SOR) ---- */

void GaussSeidel9::redBlackSweep(const QVector<QVector<double>>& A,
                                  QVector<double>& x,
                                  const QVector<double>& b)
{
    int n = x.size();
    // Two passes: red (even), black (odd)
    for (int pass = 0; pass < 2; ++pass) {
        for (int i = pass; i < n; i += 2) {
            double sigma = 0.0;
            double diag = 1.0;
            for (int j = 0; j < n; ++j) {
                if (j == i) {
                    diag = A[i][j];
                } else {
                    sigma += A[i][j] * x[j];
                }
            }
            if (qAbs(diag) > 1e-300) {
                double xNew = (b[i] - sigma) / diag;
                x[i] = x[i] + m_omega * (xNew - x[i]);
            }
        }
    }
}

/* ---- Poisson sweep with 5-point stencil ---- */

void GaussSeidel9::poissonSweep(QVector<QVector<double>>& u,
                                 const QVector<QVector<double>>& rhs,
                                 int nx, int ny, double dx, double dy)
{
    double idx2 = 1.0 / (dx * dx);
    double idy2 = 1.0 / (dy * dy);
    double diag = 2.0 * (idx2 + idy2);

    // Red-black ordering based on (i+j) parity
    for (int pass = 0; pass < 2; ++pass) {
        for (int j = 1; j < ny - 1; ++j) {
            for (int i = 1; i < nx - 1; ++i) {
                if ((i + j) % 2 != pass) continue;

                double xNew = (idx2 * (u[j][i - 1] + u[j][i + 1]) +
                               idy2 * (u[j - 1][i] + u[j + 1][i]) -
                               rhs[j][i]) / diag;

                u[j][i] = u[j][i] + m_omega * (xNew - u[j][i]);
            }
        }
    }
}

/* ---- Solve general Ax = b ---- */

GaussSeidel9::SolveResult GaussSeidel9::solve(const QVector<QVector<double>>& A,
                                                const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    int n = b.size();

    if (n == 0) return result;

    // Initialize x to zero
    result.solution.resize(n, 0.0);

    int iter = 0;
    for (; iter < m_maxIterations; ++iter) {
        redBlackSweep(A, result.solution, b);

        double res = computeResidual(A, result.solution, b);
        if (res < m_tolerance) {
            result.converged = true;
            break;
        }
    }

    result.iterations = iter;
    result.finalResidual = computeResidual(A, result.solution, b);
    result.solveTimeMs = timer.elapsed();

    m_stats.totalSolves++;
    m_stats.problemSize = n;
    m_iterSum += iter;
    m_stats.avgIterations = static_cast<double>(m_iterSum) / m_stats.totalSolves;
    m_timeSum += result.solveTimeMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(n, iter, result.finalResidual, result.solveTimeMs);
    return result;
}

/* ---- Solve 2D Poisson equation ---- */

GaussSeidel9::SolveResult GaussSeidel9::solvePoisson2D(
    const QVector<QVector<double>>& rhs, int nx, int ny, double dx, double dy)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;

    // Initialize u to zero
    QVector<QVector<double>> u(ny, QVector<double>(nx, 0.0));

    int iter = 0;
    double prevRes = 1e300;

    for (; iter < m_maxIterations; ++iter) {
        poissonSweep(u, rhs, nx, ny, dx, dy);

        // Compute residual for 5-point stencil
        double maxRes = 0.0;
        double idx2 = 1.0 / (dx * dx);
        double idy2 = 1.0 / (dy * dy);
        double diag = 2.0 * (idx2 + idy2);

        for (int j = 1; j < ny - 1; ++j) {
            for (int i = 1; i < nx - 1; ++i) {
                double res = qAbs(
                    diag * u[j][i] - idx2 * (u[j][i - 1] + u[j][i + 1]) -
                    idy2 * (u[j - 1][i] + u[j + 1][i]) + rhs[j][i]);
                maxRes = qMax(maxRes, res);
            }
        }

        if (maxRes < m_tolerance) {
            result.converged = true;
            break;
        }
        prevRes = maxRes;
    }

    // Flatten to 1D solution vector
    result.solution.reserve(nx * ny);
    for (int j = 0; j < ny; ++j)
        for (int i = 0; i < nx; ++i)
            result.solution.append(u[j][i]);

    result.iterations = iter;
    result.solveTimeMs = timer.elapsed();

    int totalSize = nx * ny;
    m_stats.totalSolves++;
    m_stats.problemSize = totalSize;
    m_iterSum += iter;
    m_stats.avgIterations = static_cast<double>(m_iterSum) / m_stats.totalSolves;
    m_timeSum += result.solveTimeMs;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveDone(totalSize, iter, result.finalResidual, result.solveTimeMs);
    return result;
}

/* ---- Reset ---- */

void GaussSeidel9::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_iterSum = 0;
}
