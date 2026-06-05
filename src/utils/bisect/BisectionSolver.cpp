/**
 * @file BisectionSolver.cpp
 * @brief 二分法/割线法/试位法实现
 */

#include "utils/bisect/BisectionSolver.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
BisectionSolver::BisectionSolver(QObject* parent)
    : QObject(parent)
{
}

/** @brief 二分法求根 */
double BisectionSolver::bisection(Func f, double a, double b,
                                   double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    double fa = f(a);
    double fb = f(b);

    /* 如果端点已是根 */
    if (std::abs(fa) < tol) {
        m_stats.totalSolves++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit rootFound(a, 0);
        return a;
    }
    if (std::abs(fb) < tol) {
        m_stats.totalSolves++;
        m_timeSum += timer.elapsed();
        m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
        emit rootFound(b, 0);
        return b;
    }

    double root = a;
    int iter = 0;
    for (iter = 0; iter < maxIter; ++iter) {
        root = (a + b) / 2.0;
        double fr = f(root);

        if (std::abs(fr) < tol || (b - a) / 2.0 < tol)
            break;

        if (fa * fr < 0.0) {
            b = root;
        } else {
            a = root;
            fa = fr;
        }
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit rootFound(root, iter + 1);
    return root;
}

/** @brief 割线法求根 */
double BisectionSolver::secant(Func f, double x0, double x1,
                                double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    double f0 = f(x0);
    int iter = 0;

    for (iter = 0; iter < maxIter; ++iter) {
        double f1 = f(x1);

        if (std::abs(f1) < tol) {
            x0 = x1;
            break;
        }

        double denom = f1 - f0;
        if (std::abs(denom) < 1e-300) break;

        double x2 = x1 - f1 * (x1 - x0) / denom;
        x0 = x1;
        f0 = f1;
        x1 = x2;
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit rootFound(x1, iter + 1);
    return x1;
}

/** @brief 试位法求根 */
double BisectionSolver::falsePosition(Func f, double a, double b,
                                       double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    double fa = f(a);
    double fb = f(b);
    double root = a;
    int iter = 0;

    for (iter = 0; iter < maxIter; ++iter) {
        double denom = fb - fa;
        if (std::abs(denom) < 1e-300) break;

        root = (a * fb - b * fa) / denom;
        double fr = f(root);

        if (std::abs(fr) < tol) break;

        if (fa * fr < 0.0) {
            b = root;
            fb = fr;
        } else {
            a = root;
            fa = fr;
        }
    }

    m_stats.totalSolves++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;
    emit rootFound(root, iter + 1);
    return root;
}

/** @brief 重置统计 */
void BisectionSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
