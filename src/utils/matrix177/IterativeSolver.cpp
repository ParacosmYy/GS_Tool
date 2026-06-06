/**
 * @file IterativeSolver.cpp
 * @brief IterativeSolver 实现
 *
 * 实现迭代线性求解器：Jacobi、Gauss-Seidel、SOR，含收敛监控。
 */

#include "utils/matrix177/IterativeSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction / Destruction ---- */

IterativeSolver::IterativeSolver(QObject *parent)
    : QObject(parent)
{
}

IterativeSolver::~IterativeSolver() = default;

/* ---- Configuration ---- */

void IterativeSolver::setMethod(Method m) { m_method = m; }
void IterativeSolver::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void IterativeSolver::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }
void IterativeSolver::setRelaxation(double omega) { m_omega = qBound(0.0, omega, 2.0); }

/* ---- Residual norm ||Ax - b||_inf ---- */

double IterativeSolver::residualNorm(const QVector<QVector<double>>& A,
                                      const QVector<double>& b,
                                      const QVector<double>& x)
{
    int n = b.size();
    double maxR = 0.0;
    for (int i = 0; i < n; ++i) {
        double ax = 0.0;
        for (int j = 0; j < n; ++j)
            ax += A[i][j] * x[j];
        double r = qAbs(ax - b[i]);
        if (r > maxR) maxR = r;
    }
    return maxR;
}

/* ---- Jacobi iteration ---- */

QVector<double> IterativeSolver::solveJacobi(const QVector<QVector<double>>& A,
                                              const QVector<double>& b,
                                              const QVector<double>& x0)
{
    int n = b.size();
    QVector<double> x = x0.isEmpty() ? QVector<double>(n, 0.0) : x0;
    QVector<double> xNew(n);

    m_conv.residualHistory.clear();

    for (int iter = 0; iter < m_maxIter; ++iter) {
        for (int i = 0; i < n; ++i) {
            double sigma = 0.0;
            for (int j = 0; j < n; ++j) {
                if (j != i) sigma += A[i][j] * x[j];
            }
            if (qAbs(A[i][i]) < 1e-30) return x;
            xNew[i] = (b[i] - sigma) / A[i][i];
        }
        x = xNew;

        double res = residualNorm(A, b, x);
        m_conv.residualHistory.append(res);

        if (iter % 50 == 0)
            emit iterationProgress(iter, res);

        if (res < m_tol) {
            m_conv.converged = true;
            m_conv.iterations = iter + 1;
            m_conv.finalResidual = res;
            return x;
        }
    }

    m_conv.converged = false;
    m_conv.iterations = m_maxIter;
    m_conv.finalResidual = residualNorm(A, b, x);
    return x;
}

/* ---- Gauss-Seidel iteration ---- */

QVector<double> IterativeSolver::solveGaussSeidel(const QVector<QVector<double>>& A,
                                                    const QVector<double>& b,
                                                    const QVector<double>& x0)
{
    int n = b.size();
    QVector<double> x = x0.isEmpty() ? QVector<double>(n, 0.0) : x0;

    m_conv.residualHistory.clear();

    for (int iter = 0; iter < m_maxIter; ++iter) {
        for (int i = 0; i < n; ++i) {
            double sigma = 0.0;
            for (int j = 0; j < n; ++j) {
                if (j != i) sigma += A[i][j] * x[j];
            }
            if (qAbs(A[i][i]) < 1e-30) return x;
            x[i] = (b[i] - sigma) / A[i][i];
        }

        double res = residualNorm(A, b, x);
        m_conv.residualHistory.append(res);

        if (iter % 50 == 0)
            emit iterationProgress(iter, res);

        if (res < m_tol) {
            m_conv.converged = true;
            m_conv.iterations = iter + 1;
            m_conv.finalResidual = res;
            return x;
        }
    }

    m_conv.converged = false;
    m_conv.iterations = m_maxIter;
    m_conv.finalResidual = residualNorm(A, b, x);
    return x;
}

/* ---- SOR (Successive Over-Relaxation) ---- */

QVector<double> IterativeSolver::solveSOR(const QVector<QVector<double>>& A,
                                           const QVector<double>& b,
                                           const QVector<double>& x0)
{
    int n = b.size();
    QVector<double> x = x0.isEmpty() ? QVector<double>(n, 0.0) : x0;

    m_conv.residualHistory.clear();

    for (int iter = 0; iter < m_maxIter; ++iter) {
        for (int i = 0; i < n; ++i) {
            double sigma = 0.0;
            for (int j = 0; j < n; ++j) {
                if (j != i) sigma += A[i][j] * x[j];
            }
            if (qAbs(A[i][i]) < 1e-30) return x;
            double gs = (b[i] - sigma) / A[i][i];
            x[i] = (1.0 - m_omega) * x[i] + m_omega * gs;
        }

        double res = residualNorm(A, b, x);
        m_conv.residualHistory.append(res);

        if (iter % 50 == 0)
            emit iterationProgress(iter, res);

        if (res < m_tol) {
            m_conv.converged = true;
            m_conv.iterations = iter + 1;
            m_conv.finalResidual = res;
            return x;
        }
    }

    m_conv.converged = false;
    m_conv.iterations = m_maxIter;
    m_conv.finalResidual = residualNorm(A, b, x);
    return x;
}

/* ---- Main solve dispatch ---- */

QVector<double> IterativeSolver::solve(const QVector<QVector<double>>& A,
                                        const QVector<double>& b,
                                        const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    m_conv = ConvergenceInfo{};

    if (n == 0 || A.size() != n) return {};

    /* Verify square matrix */
    for (int i = 0; i < n; ++i)
        if (A[i].size() != n) return {};

    QVector<double> result;
    switch (m_method) {
    case Jacobi:
        result = solveJacobi(A, b, x0);
        break;
    case GaussSeidel:
        result = solveGaussSeidel(A, b, x0);
        break;
    case SOR:
        result = solveSOR(A, b, x0);
        break;
    }

    m_stats.totalSolves++;
    m_stats.totalIterations += m_conv.iterations;
    m_stats.lastN = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(m_conv.iterations, m_conv.finalResidual, m_conv.converged);
    return result;
}

/* ---- Statistics ---- */

void IterativeSolver::resetStatistics()
{
    m_stats = Stats{};
    m_conv = ConvergenceInfo{};
    m_timeSum = 0.0;
}
