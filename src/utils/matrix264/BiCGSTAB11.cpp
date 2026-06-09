/**
 * @file BiCGSTAB11.cpp
 * @brief BiCGSTAB11 实现
 *
 * 实现双共轭梯度稳定法：GPBi-CGS变体复合步残差平滑不定系统鲁棒求解。
 */

#include "utils/matrix264/BiCGSTAB11.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>

/* ---- Construction / Destruction ---- */

BiCGSTAB11::BiCGSTAB11(QObject *parent)
    : QObject(parent) {}
BiCGSTAB11::~BiCGSTAB11() = default;

/* ---- Configuration ---- */

void BiCGSTAB11::setMaxIterations(int iters) { m_maxIter = qMax(1, iters); }
void BiCGSTAB11::setTolerance(double tol) { m_tol = qMax(1e-15, tol); }

/* ---- Linear algebra helpers ---- */

double BiCGSTAB11::dot(const QVector<double>& a, const QVector<double>& b)
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double BiCGSTAB11::norm(const QVector<double>& v)
{
    return qSqrt(dot(v, v));
}

void BiCGSTAB11::axpy(double alpha, const QVector<double>& x, QVector<double>& y)
{
    int n = qMin(x.size(), y.size());
    for (int i = 0; i < n; ++i) y[i] += alpha * x[i];
}

void BiCGSTAB11::scale(double alpha, QVector<double>& x)
{
    for (int i = 0; i < x.size(); ++i) x[i] *= alpha;
}

/* ---- Solve ---- */

QVector<double> BiCGSTAB11::solve(
    const QVector<double>& b,
    const QVector<double>& x0,
    const std::function<QVector<double>(const QVector<double>&)>& matvec)
{
    QElapsedTimer timer;
    timer.start();

    int n = b.size();
    if (n == 0) return {};
    m_residualHistory.clear();

    // Initialize: x = x0, r = b - A*x0
    QVector<double> x = x0;
    if (x.size() != n) x.resize(n, 0.0);
    QVector<double> r = matvec(x);
    for (int i = 0; i < n; ++i) r[i] = b[i] - r[i];

    double rNorm0 = norm(r);
    if (rNorm0 < m_tol) {
        m_stats.finalResidual = rNorm0;
        m_stats.iterationsUsed = 0;
        m_stats.matrixSize = n;
        return x;
    }

    // Choose r~ = r (shadow residual)
    QVector<double> rShadow = r;
    QVector<double> p = r;
    double rho = dot(rShadow, r);

    // GPBi-CGS variant with residual smoothing
    QVector<double> xSmooth = x;      // Smoothed solution
    double smoothNorm = rNorm0;

    for (int iter = 0; iter < m_maxIter; ++iter) {
        // Matrix-vector product: v = A*p
        QVector<double> v = matvec(p);

        // Compute sigma = (r~, v)
        double sigma = dot(rShadow, v);
        if (qAbs(sigma) < 1e-30 * qAbs(rho)) {
            // Breakdown protection: restart with current residual
            r = matvec(x);
            for (int i = 0; i < n; ++i) r[i] = b[i] - r[i];
            rShadow = r;
            p = r;
            rho = dot(rShadow, r);
            continue;
        }

        // alpha = rho / sigma  (step size)
        double alpha = rho / sigma;

        // s = r - alpha*v
        QVector<double> s = r;
        axpy(-alpha, v, s);

        // Composite step: check if |s| is small enough
        double sNorm = norm(s);
        if (sNorm < m_tol * rNorm0) {
            // Converged after first half-step
            axpy(alpha, p, x);
            r = s;
            m_residualHistory.append(sNorm / rNorm0);

            // Residual smoothing
            if (sNorm < smoothNorm) {
                xSmooth = x;
                smoothNorm = sNorm;
            }
            break;
        }

        // Second matrix-vector: t = A*s
        QVector<double> t = matvec(s);

        // omega = (s, t) / (t, t)
        double omega = dot(s, t) / dot(t, t);
        if (qAbs(omega) < 1e-30) omega = 1.0;
        // Stability: clamp omega
        omega = qBound(0.01, omega, 100.0);

        // Update solution: x = x + alpha*p + omega*s
        axpy(alpha, p, x);
        axpy(omega, s, x);

        // Update residual: r = s - omega*t
        r = s;
        axpy(-omega, t, r);

        double rNorm = norm(r);
        m_residualHistory.append(rNorm / rNorm0);

        // Residual smoothing: keep the best solution seen so far
        if (rNorm < smoothNorm) {
            xSmooth = x;
            smoothNorm = rNorm;
        }

        double elapsed = timer.elapsed();
        emit iterationCompleted(iter, rNorm / rNorm0, elapsed);

        if (rNorm / rNorm0 < m_tol) break;

        // Compute new rho and update p (GPBi-CGS variant)
        double rhoNew = dot(rShadow, r);
        double beta = (rhoNew / rho) * (alpha / omega);
        rho = rhoNew;

        // p = r + beta*(p - omega*v)
        for (int i = 0; i < n; ++i)
            p[i] = r[i] + beta * (p[i] - omega * v[i]);
    }

    double elapsed = timer.elapsed();
    // Use smoothed solution if it's better
    double finalRes = norm(r);
    if (smoothNorm < finalRes) {
        x = xSmooth;
        finalRes = smoothNorm;
    }

    m_stats.matrixSize = n;
    m_stats.iterationsUsed = m_residualHistory.size();
    m_stats.finalResidual = finalRes;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    emit solveCompleted(m_stats.iterationsUsed, finalRes, elapsed);
    return x;
}

/* ---- Residual history ---- */

QVector<double> BiCGSTAB11::residualHistory() const { return m_residualHistory; }

/* ---- Reset ---- */

void BiCGSTAB11::resetStatistics()
{
    m_residualHistory.clear();
    m_stats = Stats{};
    m_timeSum = 0.0;
}
