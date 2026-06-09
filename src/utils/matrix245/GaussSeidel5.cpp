/**
 * @file GaussSeidel5.cpp
 * @brief GaussSeidel5 实现
 *
 * 实现高斯-赛德尔迭代：逐次超松弛与红黑排序并行稀疏求解。
 */

#include "utils/matrix245/GaussSeidel5.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

GaussSeidel5::GaussSeidel5(QObject *parent) : QObject(parent) {}
GaussSeidel5::~GaussSeidel5() = default;

/* ---- Configuration ---- */

void GaussSeidel5::setOmega(double omega) { m_omega = qBound(0.01, omega, 1.99); }
void GaussSeidel5::setTolerance(double tol) { m_tolerance = qMax(1e-15, tol); }
void GaussSeidel5::setMaxIterations(int maxIter) { m_maxIter = qMax(1, maxIter); }

/* ---- Compute residual ---- */

double GaussSeidel5::computeResidual(const QVector<QVector<double>>& A,
                                      const QVector<double>& b,
                                      const QVector<double>& x)
{
    int n = x.size();
    double maxRes = 0.0;
    for (int i = 0; i < n; ++i) {
        double sum = -b[i];
        for (int j = 0; j < n; ++j)
            sum += A[i][j] * x[j];
        maxRes = qMax(maxRes, qAbs(sum));
    }
    return maxRes;
}

/* ---- Build red-black coloring ---- */

QVector<int> GaussSeidel5::buildColoring(int n) const
{
    // Simple 2-color: even indices = red(0), odd = black(1)
    QVector<int> color(n);
    for (int i = 0; i < n; ++i)
        color[i] = i % 2;
    return color;
}

/* ---- Standard Gauss-Seidel with SOR ---- */

QVector<double> GaussSeidel5::solve(const QVector<QVector<double>>& A,
                                     const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0 || A.size() != n) return {};

    QVector<double> x(n, 0.0);
    m_residualHistory.clear();
    m_stats.matrixSize = n;
    m_stats.omega = m_omega;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        for (int i = 0; i < n; ++i) {
            double sigma = 0.0;
            for (int j = 0; j < n; ++j) {
                if (j != i) sigma += A[i][j] * x[j];
            }
            double xNew = (b[i] - sigma) / A[i][i];
            x[i] = x[i] + m_omega * (xNew - x[i]);
        }

        double res = computeResidual(A, b, x);
        m_residualHistory.append(res);

        if (iter % 10 == 0)
            emit iterationCompleted(iter, res);

        if (res < m_tolerance) {
            m_stats.numIterations = iter + 1;
            m_stats.finalResidual = res;
            break;
        }
        m_stats.numIterations = iter + 1;
        m_stats.finalResidual = res;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_stats.numIterations, m_stats.finalResidual, timer.elapsed());
    return x;
}

/* ---- Red-black Gauss-Seidel with SOR ---- */

QVector<double> GaussSeidel5::solveRedBlack(const QVector<QVector<double>>& A,
                                              const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0 || A.size() != n) return {};

    QVector<double> x(n, 0.0);
    QVector<int> color = buildColoring(n);
    m_residualHistory.clear();
    m_stats.matrixSize = n;
    m_stats.omega = m_omega;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Update red (color 0) then black (color 1)
        for (int c = 0; c < 2; ++c) {
            for (int i = 0; i < n; ++i) {
                if (color[i] != c) continue;
                double sigma = 0.0;
                for (int j = 0; j < n; ++j) {
                    if (j != i) sigma += A[i][j] * x[j];
                }
                double xNew = (b[i] - sigma) / A[i][i];
                x[i] = x[i] + m_omega * (xNew - x[i]);
            }
        }

        double res = computeResidual(A, b, x);
        m_residualHistory.append(res);

        if (iter % 10 == 0)
            emit iterationCompleted(iter, res);

        if (res < m_tolerance) {
            m_stats.numIterations = iter + 1;
            m_stats.finalResidual = res;
            break;
        }
        m_stats.numIterations = iter + 1;
        m_stats.finalResidual = res;
    }

    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_stats.numIterations, m_stats.finalResidual, timer.elapsed());
    return x;
}

/* ---- Accessors ---- */

QVector<double> GaussSeidel5::residualHistory() const { return m_residualHistory; }

/* ---- Reset ---- */

void GaussSeidel5::resetStatistics()
{
    m_residualHistory.clear();
    m_stats = Stats{}; m_timeSum = 0.0;
}
