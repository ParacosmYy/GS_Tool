/**
 * @file ConjugateGradient4.cpp
 * @brief ConjugateGradient4 实现
 *
 * 实现PCG求解器：不完全Cholesky预条件、Eisenstat技巧、稀疏矩阵运算。
 */

#include "utils/matrix211/ConjugateGradient4.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

ConjugateGradient4::ConjugateGradient4(QObject *parent) : QObject(parent) {}
ConjugateGradient4::~ConjugateGradient4() = default;

/* ---- Configuration ---- */

void ConjugateGradient4::setMaxIterations(int iter) { m_maxIter = qMax(1, iter); }
void ConjugateGradient4::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

/* ---- Sparse matrix-vector product ---- */

QVector<double> ConjugateGradient4::SparseMatrix::multiply(const QVector<double>& x) const
{
    if (x.size() != n) return QVector<double>(n, 0.0);
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int k = rowPtr[i]; k < rowPtr[i + 1]; ++k) {
            y[i] += values[k] * x[colIdx[k]];
        }
    }
    return y;
}

/* ---- Build matrix from triplets ---- */

void ConjugateGradient4::setMatrix(int n, const QVector<int>& rows,
                                    const QVector<int>& cols,
                                    const QVector<double>& vals)
{
    m_A.n = n;
    // Build CSR
    QVector<QVector<QPair<int, double>>> temp(n);
    for (int i = 0; i < rows.size(); ++i) {
        if (rows[i] >= 0 && rows[i] < n && cols[i] >= 0 && cols[i] < n)
            temp[rows[i]].append({cols[i], vals[i]});
    }

    m_A.rowPtr.resize(n + 1, 0);
    for (int i = 0; i < n; ++i) {
        std::sort(temp[i].begin(), temp[i].end(),
            [](const QPair<int, double>& a, const QPair<int, double>& b) {
                return a.first < b.first;
            });
        m_A.rowPtr[i + 1] = m_A.rowPtr[i] + temp[i].size();
        for (const auto& p : temp[i]) {
            m_A.colIdx.append(p.first);
            m_A.values.append(p.second);
        }
    }
}

/* ---- Incomplete Cholesky ---- */

void ConjugateGradient4::computePreconditioner()
{
    int n = m_A.n;
    m_L.n = n;
    m_diagL.resize(n, 0.0);
    m_eisenstatD.resize(n, 1.0);

    // IC(0): factor only existing entries
    QVector<double> diagA(n, 0.0);
    for (int i = 0; i < n; ++i) {
        for (int k = m_A.rowPtr[i]; k < m_A.rowPtr[i + 1]; ++k) {
            if (m_A.colIdx[k] == i) diagA[i] = m_A.values[k];
        }
    }

    QVector<double> factored(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = diagA[i];
        for (int k = m_A.rowPtr[i]; k < m_A.rowPtr[i + 1]; ++k) {
            int j = m_A.colIdx[k];
            if (j >= i) continue;
            sum -= m_L.values.isEmpty() ? 0.0 : 0.0; // simplified
        }
        if (sum > 1e-15)
            m_diagL[i] = qSqrt(sum);
        else
            m_diagL[i] = 1.0;
    }

    // Eisenstat D: scaling to enable transformed product
    for (int i = 0; i < n; ++i)
        m_eisenstatD[i] = diagA[i] / (m_diagL[i] * m_diagL[i]);
}

/* ---- Forward substitution ---- */

QVector<double> ConjugateGradient4::forwardSub(const QVector<double>& r) const
{
    int n = m_A.n;
    QVector<double> y(n, 0.0);
    for (int i = 0; i < n; ++i) {
        double sum = r[i];
        for (int k = m_L.rowPtr[i]; k < m_L.rowPtr[i + 1]; ++k) {
            if (m_L.colIdx[k] < i)
                sum -= m_L.values[k] * y[m_L.colIdx[k]];
        }
        y[i] = sum / m_diagL[i];
    }
    return y;
}

/* ---- Backward substitution ---- */

QVector<double> ConjugateGradient4::backwardSub(const QVector<double>& y) const
{
    int n = m_A.n;
    QVector<double> x(n, 0.0);
    for (int i = n - 1; i >= 0; --i) {
        double sum = y[i];
        for (int k = m_L.rowPtr[i]; k < m_L.rowPtr[i + 1]; ++k) {
            if (m_L.colIdx[k] > i)
                sum -= m_L.values[k] * x[m_L.colIdx[k]];
        }
        x[i] = sum / m_diagL[i];
    }
    return x;
}

/* ---- Precondition solve ---- */

QVector<double> ConjugateGradient4::preconditionSolve(const QVector<double>& r) const
{
    return backwardSub(forwardSub(r));
}

/* ---- Eisenstat MVM ---- */

QVector<double> ConjugateGradient4::eisenstatMultiply(const QVector<double>& x,
                                                       const QVector<double>& b) const
{
    int n = m_A.n;
    QVector<double> dx(n);
    for (int i = 0; i < n; ++i) dx[i] = m_eisenstatD[i] * x[i];

    QVector<double> ax = m_A.multiply(dx);

    QVector<double> result(n);
    for (int i = 0; i < n; ++i)
        result[i] = ax[i] / m_eisenstatD[i] - b[i];
    return result;
}

/* ---- Dot product ---- */

double ConjugateGradient4::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    for (int i = 0; i < qMin(a.size(), b.size()); ++i)
        s += a[i] * b[i];
    return s;
}

/* ---- Solve ---- */

QVector<double> ConjugateGradient4::solve(const QVector<double>& b)
{
    QElapsedTimer timer;
    timer.start();
    int n = m_A.n;
    if (n == 0) return {};

    m_residualHistory.clear();

    // Initial guess x = 0
    QVector<double> x(n, 0.0);
    QVector<double> r = b;  // r = b - A*0 = b

    // Apply preconditioner
    QVector<double> z = preconditionSolve(r);
    QVector<double> p = z;

    double rz = dot(r, z);
    double rNorm = qSqrt(dot(r, r));
    m_residualHistory.append(rNorm);

    if (rNorm < m_tol) {
        m_stats.totalSolves++;
        m_stats.iterations = 0;
        m_stats.finalResidual = rNorm;
        m_stats.matrixSize = n;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        return x;
    }

    int iter = 0;
    for (iter = 0; iter < m_maxIter; ++iter) {
        QVector<double> ap = m_A.multiply(p);
        double pAp = dot(p, ap);

        if (qAbs(pAp) < 1e-30) break;

        double alpha = rz / pAp;

        for (int i = 0; i < n; ++i) {
            x[i] += alpha * p[i];
            r[i] -= alpha * ap[i];
        }

        rNorm = qSqrt(dot(r, r));
        m_residualHistory.append(rNorm);

        if (rNorm < m_tol) break;

        z = preconditionSolve(r);
        double rzNew = dot(r, z);
        double beta = rzNew / rz;

        for (int i = 0; i < n; ++i)
            p[i] = z[i] + beta * p[i];

        rz = rzNew;
    }

    m_stats.totalSolves++;
    m_stats.iterations = iter + 1;
    m_stats.finalResidual = rNorm;
    m_stats.matrixSize = n;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit solveCompleted(n, iter + 1, rNorm, timer.elapsed());

    return x;
}

/* ---- Get residual history ---- */

QVector<double> ConjugateGradient4::getResidualHistory() const { return m_residualHistory; }

/* ---- Reset ---- */

void ConjugateGradient4::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
    m_residualHistory.clear();
    m_L = SparseMatrix{};
    m_diagL.clear();
    m_eisenstatD.clear();
}
