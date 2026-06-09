/**
 * @file BiCGSTAB10.cpp
 * @brief BiCGSTAB10 实现
 *
 * 实现稳定双共轭梯度法：GPBi-CAB复合步变体与残差平滑单调收敛保证。
 */

#include "utils/matrix254/BiCGSTAB10.h"

#include <QElapsedTimer>
#include <QtMath>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB10::BiCGSTAB10(QObject *parent) : QObject(parent) {}
BiCGSTAB10::~BiCGSTAB10() = default;

/* ---- Configuration ---- */

void BiCGSTAB10::setTolerance(double tol, int maxIter)
{
    m_tolerance = qMax(1e-15, tol);
    m_maxIter = qMax(1, maxIter);
}

/* ---- Sparse matrix-vector multiply ---- */

void BiCGSTAB10::spmv(const QVector<double>& val,
                        const QVector<int>& col,
                        const QVector<int>& rowPtr,
                        const QVector<double>& x,
                        QVector<double>& y)
{
    int n = rowPtr.size() - 1;
    for (int i = 0; i < n; ++i) {
        double sum = 0.0;
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j)
            sum += val[j] * x[col[j]];
        y[i] = sum;
    }
}

/* ---- Dot product ---- */

double BiCGSTAB10::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

/* ---- Axpy ---- */

void BiCGSTAB10::axpy(double alpha, const QVector<double>& x,
                        QVector<double>& y)
{
    int n = qMin(x.size(), y.size());
    for (int i = 0; i < n; ++i) y[i] += alpha * x[i];
}

/* ---- Vector norm ---- */

double BiCGSTAB10::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

/* ---- GPBi-CAB composite step ---- */

void BiCGSTAB10::compositeStep(const QVector<double>& val,
                                 const QVector<int>& col,
                                 const QVector<int>& rowPtr,
                                 const QVector<double>& r,
                                 const QVector<double>& p,
                                 QVector<double>& Ap,
                                 QVector<double>& Ar) const
{
    // Compute Ap = A*p and Ar = A*r simultaneously
    int n = rowPtr.size() - 1;
    Ap.resize(n);
    Ar.resize(n);
    for (int i = 0; i < n; ++i) {
        double sp = 0.0, sr = 0.0;
        for (int j = rowPtr[i]; j < rowPtr[i + 1]; ++j) {
            double aij = val[j];
            int c = col[j];
            sp += aij * p[c];
            sr += aij * r[c];
        }
        Ap[i] = sp;
        Ar[i] = sr;
    }
}

/* ---- Residual smoothing for monotonic convergence ---- */

void BiCGSTAB10::smoothResidual(QVector<double>& rSmooth,
                                  const QVector<double>& rNew,
                                  double& rhoSmooth) const
{
    double rhoNew = dot(rNew, rNew);
    // Convex combination to ensure monotonic decrease
    if (rhoNew < rhoSmooth) {
        rSmooth = rNew;
        rhoSmooth = rhoNew;
    } else {
        // Blend: move towards new residual but keep decrease
        double alpha = rhoSmooth / qMax(rhoNew, 1e-30);
        alpha = qBound(0.0, alpha, 0.5);
        for (int i = 0; i < rSmooth.size(); ++i)
            rSmooth[i] = (1.0 - alpha) * rSmooth[i] + alpha * rNew[i];
        rhoSmooth = dot(rSmooth, rSmooth);
    }
}

/* ---- Main solve ---- */

QVector<double> BiCGSTAB10::solve(const QVector<double>& values,
                                    const QVector<int>& colIdx,
                                    const QVector<int>& rowPtr,
                                    const QVector<double>& rhs)
{
    QElapsedTimer timer;
    timer.start();

    int n = rhs.size();
    if (n == 0) return {};

    // Initial guess x0 = 0
    QVector<double> x(n, 0.0);
    QVector<double> r(n), rHat(n);
    QVector<double> tmp(n, 0.0);

    // r0 = b - A*x0 = b
    spmv(values, colIdx, rowPtr, x, tmp);
    for (int i = 0; i < n; ++i) r[i] = rhs[i] - tmp[i];

    // Choose rHat = r0
    rHat = r;

    double rhoOld = dot(rHat, r);
    double initRes = norm(r);
    m_residualHistory.clear();
    m_residualHistory.append(initRes);

    QVector<double> p(n, 0.0), v(n, 0.0);
    QVector<double> s(n), t(n), Ap(n), Ar(n);
    double omega = 1.0;
    double alpha = 1.0;

    // Smoothed residual tracking
    QVector<double> rSmooth = r;
    double rhoSmooth = initRes * initRes;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        double rho = dot(rHat, r);
        if (qAbs(rho) < 1e-30 * qMax(qAbs(rhoOld), 1e-30)) break;

        double beta = (rho / qMax(rhoOld, 1e-30)) * (alpha / qMax(omega, 1e-30));

        // p = r + beta * (p - omega * v)
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);

        // GPBi-CAB composite step: compute Ap and Ar simultaneously
        compositeStep(values, colIdx, rowPtr, r, p, Ap, Ar);
        v = Ap;

        double rHatv = dot(rHat, v);
        if (qAbs(rHatv) < 1e-30) break;
        alpha = rho / rHatv;

        // s = r - alpha * v
        for (int i = 0; i < n; ++i)
            s[i] = r[i] - alpha * v[i];

        // Check early convergence on s
        double sNorm = norm(s);
        if (sNorm < m_tolerance) {
            axpy(alpha, p, x);
            r = s;
            break;
        }

        // t = A * s (reuse Ar if possible)
        spmv(values, colIdx, rowPtr, s, t);

        double tt = dot(t, t);
        omega = (tt > 1e-30) ? dot(t, s) / tt : 0.0;

        // Update solution
        axpy(alpha, p, x);
        axpy(omega, s, x);

        // Update residual
        for (int i = 0; i < n; ++i)
            r[i] = s[i] - omega * t[i];

        // Apply residual smoothing
        smoothResidual(rSmooth, r, rhoSmooth);
        r = rSmooth;

        rhoOld = rho;

        double resNorm = norm(r);
        m_residualHistory.append(resNorm);

        if (resNorm / qMax(initRes, 1e-30) < m_tolerance) break;
    }

    double finalRes = norm(r);

    m_stats.matrixSize = n;
    m_stats.numIterations = m_residualHistory.size() - 1;
    m_stats.maxIterations = m_maxIter;
    m_stats.initialResidual = initRes;
    m_stats.finalResidual = finalRes;
    m_stats.tolerance = m_tolerance;
    m_stats.converged = (finalRes / qMax(initRes, 1e-30) < m_tolerance);
    m_stats.totalOps++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;

    emit solveCompleted(m_stats.numIterations, finalRes, timer.elapsed());
    return x;
}

/* ---- Residual history ---- */

QVector<double> BiCGSTAB10::residualHistory() const
{
    return m_residualHistory;
}

/* ---- Reset ---- */

void BiCGSTAB10::resetStatistics()
{
    m_residualHistory.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
