/**
 * @file ConjugateGradient8.cpp
 * @brief ConjugateGradient8 实现
 *
 * 实现共轭梯度法：Fletcher-Reeves重启强Wolfe线搜索非二次优化。
 */

#include "utils/matrix267/ConjugateGradient8.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>
#include <limits>
#include <functional>

/* ---- Construction / Destruction ---- */

ConjugateGradient8::ConjugateGradient8(QObject *parent)
    : QObject(parent) {}

ConjugateGradient8::~ConjugateGradient8() = default;

/* ---- Configuration ---- */

void ConjugateGradient8::setObjective(ObjectiveFunc obj, GradientFunc grad)
{
    m_obj = obj;
    m_grad = grad;
}

void ConjugateGradient8::setParameters(int maxIterations, double tolerance, int restartPeriod)
{
    m_maxIter = qMax(1, maxIterations);
    m_tol = qMax(1e-15, tolerance);
    m_restartPeriod = qMax(1, restartPeriod);
}

/* ---- Vector helpers ---- */

double ConjugateGradient8::dotProduct(const QVector<double>& a, const QVector<double>& b) const
{
    double s = 0.0;
    int n = qMin(a.size(), b.size());
    for (int i = 0; i < n; ++i) s += a[i] * b[i];
    return s;
}

double ConjugateGradient8::vecNorm(const QVector<double>& v) const
{
    return qSqrt(dotProduct(v, v));
}

/* ---- Fletcher-Reeves beta ---- */

double ConjugateGradient8::fletcherReevesBeta(const QVector<double>& gNew,
                                              const QVector<double>& gOld) const
{
    double denom = dotProduct(gOld, gOld);
    if (denom < 1e-30) return 0.0;
    return dotProduct(gNew, gNew) / denom;
}

/* ---- Strong Wolfe line search ---- */

double ConjugateGradient8::strongWolfeSearch(const QVector<double>& x,
                                             const QVector<double>& p,
                                             double fx,
                                             const QVector<double>& gx) const
{
    double alpha = 1.0;
    double c1 = 1e-4;   // Armijo parameter
    double c2 = 0.1;    // Strong curvature parameter
    double alphaMax = 10.0;
    double alphaPrev = 0.0;
    double fPrev = fx;

    double gp = dotProduct(gx, p);  // directional derivative
    int n = x.size();

    for (int lsIter = 0; lsIter < 40; ++lsIter) {
        // Evaluate f(x + alpha*p)
        QVector<double> xNew(n);
        for (int i = 0; i < n; ++i) xNew[i] = x[i] + alpha * p[i];
        double fNew = m_obj(xNew);

        // Check Armijo or sufficient decrease
        if (fNew > fx + c1 * alpha * gp ||
            (lsIter > 0 && fNew >= fPrev)) {
            // Zoom phase: binary search between alphaPrev and alpha
            double lo = alphaPrev, hi = alpha;
            for (int z = 0; z < 20; ++z) {
                double mid = 0.5 * (lo + hi);
                for (int i = 0; i < n; ++i) xNew[i] = x[i] + mid * p[i];
                double fMid = m_obj(xNew);
                if (fMid > fx + c1 * mid * gp) {
                    hi = mid;
                } else {
                    auto gNew = m_grad(xNew);
                    double gpMid = dotProduct(gNew, p);
                    if (qAbs(gpMid) <= -c2 * gp) return mid;
                    if (gpMid * (hi - lo) >= 0) hi = lo;
                    lo = mid;
                }
            }
            return 0.5 * (lo + hi);
        }

        // Check strong Wolfe curvature condition
        auto gNew = m_grad(xNew);
        double gpNew = dotProduct(gNew, p);
        if (qAbs(gpNew) <= -c2 * gp) return alpha;

        if (gpNew >= 0) return alpha;  // Ascent: stop

        fPrev = fNew;
        alphaPrev = alpha;
        alpha = qMin(alpha * 2.0, alphaMax);
    }
    return alpha;
}

/* ---- Minimize ---- */

QVector<double> ConjugateGradient8::minimize(const QVector<double>& x0)
{
    QElapsedTimer timer;
    timer.start();

    int n = x0.size();
    if (n == 0 || !m_obj || !m_grad) return x0;

    QVector<double> x = x0;
    QVector<double> g = m_grad(x);
    QVector<double> p(n);
    for (int i = 0; i < n; ++i) p[i] = -g[i];  // Initial search direction = -gradient

    double fx = m_obj(x);
    QVector<double> gPrev = g;
    double prevGradNorm = vecNorm(g);

    int iter = 0;
    bool converged = false;

    for (iter = 0; iter < m_maxIter; ++iter) {
        double gradNorm = vecNorm(g);
        if (gradNorm < m_tol) { converged = true; break; }

        // Line search along direction p
        double alpha = strongWolfeSearch(x, p, fx, g);

        // Update x
        for (int i = 0; i < n; ++i) x[i] += alpha * p[i];

        // Update gradient
        gPrev = g;
        g = m_grad(x);
        fx = m_obj(x);

        // Fletcher-Reeves restart or periodic restart
        double beta = 0.0;
        if ((iter + 1) % m_restartPeriod != 0) {
            beta = fletcherReevesBeta(g, gPrev);
        }

        // Update search direction
        for (int i = 0; i < n; ++i)
            p[i] = -g[i] + beta * p[i];

        // Ensure descent direction
        if (dotProduct(p, g) >= 0) {
            for (int i = 0; i < n; ++i) p[i] = -g[i];
        }

        double elapsed = timer.elapsed();
        emit iterationCompleted(iter, fx, gradNorm, elapsed);
    }

    double elapsed = timer.elapsed();
    m_stats.numDimensions = n;
    m_stats.numIterations = iter;
    m_stats.finalValue = fx;
    m_stats.finalGradientNorm = vecNorm(g);
    m_stats.converged = converged;
    m_stats.totalOps++;
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalOps;
    return x;
}

/* ---- Reset ---- */

void ConjugateGradient8::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
