/**
 * @file EigenVectorSolver.cpp
 * @brief EigenVectorSolver 实现
 *
 * 实现特征向量求解：反幂迭代、Rayleigh商迭代、线性求解器。
 */

#include "utils/matrix174/EigenVectorSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/* ---- Construction ---- */

EigenVectorSolver::EigenVectorSolver(QObject *parent) : QObject(parent) {}
EigenVectorSolver::~EigenVectorSolver() = default;

/* ---- Configuration ---- */

void EigenVectorSolver::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }
void EigenVectorSolver::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

/* ---- Helpers ---- */

double EigenVectorSolver::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

void EigenVectorSolver::normalize(QVector<double>& v)
{
    double norm = qSqrt(dot(v, v));
    if (norm < 1e-30) return;
    for (double& x : v) x /= norm;
}

QVector<double> EigenVectorSolver::matVec(const QVector<QVector<double>>& M,
                                            const QVector<double>& v)
{
    int n = M.size();
    QVector<double> result(n, 0.0);
    for (int i = 0; i < n; ++i) {
        int cols = qMin(M[i].size(), v.size());
        for (int j = 0; j < cols; ++j)
            result[i] += M[i][j] * v[j];
    }
    return result;
}

double EigenVectorSolver::rayleighQuotient(const QVector<QVector<double>>& matrix,
                                             const QVector<double>& vec)
{
    QVector<double> Mv = matVec(matrix, vec);
    double vMv = dot(vec, Mv);
    double vv = dot(vec, vec);
    if (vv < 1e-30) return 0.0;
    return vMv / vv;
}

/* ---- Linear solver (Gaussian elimination with partial pivoting) ---- */

bool EigenVectorSolver::solveLinear(QVector<QVector<double>> A,
                                      QVector<double> b, QVector<double>& x)
{
    int n = A.size();
    x.fill(0.0, n);

    /* Forward elimination with partial pivoting */
    for (int k = 0; k < n; ++k) {
        /* Find pivot */
        int maxRow = k;
        double maxVal = qAbs(A[k][k]);
        for (int i = k + 1; i < n; ++i) {
            if (qAbs(A[i][k]) > maxVal) {
                maxVal = qAbs(A[i][k]);
                maxRow = i;
            }
        }

        /* Singular check */
        if (maxVal < 1e-15) return false;

        /* Swap rows */
        if (maxRow != k) {
            std::swap(A[k], A[maxRow]);
            std::swap(b[k], b[maxRow]);
        }

        /* Eliminate below */
        for (int i = k + 1; i < n; ++i) {
            double factor = A[i][k] / A[k][k];
            for (int j = k; j < n; ++j)
                A[i][j] -= factor * A[k][j];
            b[i] -= factor * b[k];
        }
    }

    /* Back substitution */
    for (int i = n - 1; i >= 0; --i) {
        x[i] = b[i];
        for (int j = i + 1; j < n; ++j)
            x[i] -= A[i][j] * x[j];
        x[i] /= A[i][i];
    }

    return true;
}

/* ---- Inverse iteration ---- */

double EigenVectorSolver::inverseIteration(const QVector<QVector<double>>& matrix,
                                              double shift, QVector<double>& eigenVector)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return 0.0;

    /* Build (A - shift*I) */
    QVector<QVector<double>> Ashift = matrix;
    for (int i = 0; i < n; ++i)
        Ashift[i][i] -= shift;

    /* Initial vector: all ones */
    eigenVector.fill(1.0, n);
    normalize(eigenVector);

    double eigenvalue = shift;
    bool converged = false;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* Solve (A - shift*I) * y = x */
        QVector<double> y;
        if (!solveLinear(Ashift, eigenVector, y)) {
            /* Matrix singular: shift is exactly an eigenvalue */
            converged = true;
            eigenvalue = shift;
            break;
        }

        normalize(y);

        /* Check convergence: eigenvalue via Rayleigh quotient */
        double newEigenvalue = rayleighQuotient(matrix, y);
        double diff = qAbs(newEigenvalue - eigenvalue);
        eigenvalue = newEigenvalue;
        eigenVector = y;

        if (diff < m_tol) { converged = true; break; }
        m_stats.lastIterations = iter + 1;
    }

    m_stats.totalSolves++;
    m_stats.lastEigenvalue = eigenvalue;
    m_stats.lastConverged = converged;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(eigenvalue, converged);
    return eigenvalue;
}

/* ---- Rayleigh quotient iteration ---- */

double EigenVectorSolver::rayleighIteration(const QVector<QVector<double>>& matrix,
                                               const QVector<double>& initialGuess,
                                               QVector<double>& eigenVector)
{
    QElapsedTimer timer;
    timer.start();

    int n = matrix.size();
    if (n == 0) return 0.0;

    eigenVector = initialGuess;
    normalize(eigenVector);

    double eigenvalue = rayleighQuotient(matrix, eigenVector);
    bool converged = false;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        /* Build (A - mu*I) */
        QVector<QVector<double>> Ashift = matrix;
        for (int i = 0; i < n; ++i)
            Ashift[i][i] -= eigenvalue;

        /* Solve (A - mu*I) * y = x */
        QVector<double> y;
        if (!solveLinear(Ashift, eigenVector, y)) {
            converged = true;
            break;
        }

        normalize(y);

        /* Update eigenvalue */
        double newEigenvalue = rayleighQuotient(matrix, y);
        double diff = qAbs(newEigenvalue - eigenvalue);
        eigenvalue = newEigenvalue;
        eigenVector = y;

        if (diff < m_tol) { converged = true; break; }
        m_stats.lastIterations = iter + 1;
    }

    m_stats.totalSolves++;
    m_stats.lastEigenvalue = eigenvalue;
    m_stats.lastConverged = converged;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = (m_stats.totalSolves > 0)
        ? m_timeSum / m_stats.totalSolves : 0.0;

    emit solveCompleted(eigenvalue, converged);
    return eigenvalue;
}

/* ---- Statistics ---- */

void EigenVectorSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
