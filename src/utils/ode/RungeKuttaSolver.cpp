/**
 * @file RungeKuttaSolver.cpp
 * @brief ODE求解器实现 — RK4和RK45自适应
 */

#include "utils/ode/RungeKuttaSolver.h"

#include <QElapsedTimer>
#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
RungeKuttaSolver::RungeKuttaSolver(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief RK4单步推进 */
QVector<double> RungeKuttaSolver::step(const OdeFunc& func, double t,
                                        const QVector<double>& y, double dt)
{
    int n = y.size();
    QVector<double> result(n);

    /* k1 = f(t, y) */
    QVector<double> k1 = func(t, y);

    /* k2 = f(t + dt/2, y + dt*k1/2) */
    QVector<double> y2(n);
    for (int i = 0; i < n; ++i)
        y2[i] = y[i] + 0.5 * dt * k1[i];
    QVector<double> k2 = func(t + 0.5 * dt, y2);

    /* k3 = f(t + dt/2, y + dt*k2/2) */
    QVector<double> y3(n);
    for (int i = 0; i < n; ++i)
        y3[i] = y[i] + 0.5 * dt * k2[i];
    QVector<double> k3 = func(t + 0.5 * dt, y3);

    /* k4 = f(t + dt, y + dt*k3) */
    QVector<double> y4(n);
    for (int i = 0; i < n; ++i)
        y4[i] = y[i] + dt * k3[i];
    QVector<double> k4 = func(t + dt, y4);

    /* y_{n+1} = y_n + dt/6 * (k1 + 2*k2 + 2*k3 + k4) */
    for (int i = 0; i < n; ++i)
        result[i] = y[i] + (dt / 6.0) * (k1[i] + 2.0 * k2[i] + 2.0 * k3[i] + k4[i]);

    return result;
}

/** @brief RK4区间积分 */
RungeKuttaSolver::SolveResult RungeKuttaSolver::solve(
    const OdeFunc& func, double t0, double t1,
    const QVector<double>& y0, double dt)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    double t = t0;
    QVector<double> y = y0;

    result.tValues.append(t);
    result.yValues.append(y);

    int stepCount = 0;
    int n = y0.size();

    while (t < t1 - 1e-12) {
        double actualDt = qMin(dt, t1 - t);
        y = step(func, t, y, actualDt);
        t += actualDt;
        ++stepCount;

        result.tValues.append(t);
        result.yValues.append(y);

        emit stepCompleted(stepCount, t);
    }

    result.acceptedSteps = stepCount;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.totalSteps += static_cast<quint64>(stepCount);
    double total = static_cast<double>(m_stats.totalSteps);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit solveCompleted(stepCount);
    return result;
}

/** @brief RK45自适应步长积分(Dormand-Prince) */
RungeKuttaSolver::SolveResult RungeKuttaSolver::solveAdaptive(
    const OdeFunc& func, double t0, double t1,
    const QVector<double>& y0, double dtInit, double tol)
{
    QElapsedTimer timer;
    timer.start();

    SolveResult result;
    double t = t0;
    QVector<double> y = y0;
    double dt = dtInit;

    result.tValues.append(t);
    result.yValues.append(y);

    int accepted = 0;
    int rejected = 0;

    while (t < t1 - 1e-12) {
        dt = qMin(dt, t1 - t);
        if (dt < 1e-15) break;

        auto pair = step45(func, t, y, dt);
        QVector<double> y5 = pair.first;
        QVector<double> y4 = pair.second;

        /* 误差估计 */
        double errNorm = 0.0;
        for (int i = 0; i < y.size(); ++i) {
            double scale = tol * (qAbs(y[i]) + qAbs(y5[i]) + 1e-10);
            double diff = y5[i] - y4[i];
            errNorm += (diff * diff) / (scale * scale);
        }
        errNorm = qSqrt(errNorm / static_cast<double>(y.size()));

        if (errNorm <= 1.0) {
            /* 接受 */
            t += dt;
            y = y5;
            ++accepted;
            result.tValues.append(t);
            result.yValues.append(y);
            emit stepCompleted(accepted, t);

            /* 增大步长 */
            if (errNorm > 1e-10) {
                dt *= qMin(5.0, 0.9 * qPow(1.0 / errNorm, 0.2));
            } else {
                dt *= 5.0;
            }
        } else {
            /* 拒绝 */
            ++rejected;
            dt *= qMax(0.1, 0.9 * qPow(1.0 / errNorm, 0.2));
        }
    }

    result.acceptedSteps = accepted;
    result.rejectedSteps = rejected;

    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    m_stats.totalSteps += static_cast<quint64>(accepted);
    m_stats.totalRejected += static_cast<quint64>(rejected);
    double total = static_cast<double>(accepted + rejected);
    m_stats.avgProcessingTimeMs = (total > 0) ? m_timeSum / total : 0.0;

    emit solveCompleted(accepted);
    return result;
}

/** @brief 重置统计 */
void RungeKuttaSolver::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}

/** @brief Dormand-Prince RK45单步 */
QPair<QVector<double>, QVector<double>> RungeKuttaSolver::step45(
    const OdeFunc& func, double t,
    const QVector<double>& y, double dt)
{
    int n = y.size();

    /* Butcher表系数 — Dormand-Prince */
    QVector<double> k1 = func(t, y);

    QVector<double> y2(n);
    for (int i = 0; i < n; ++i) y2[i] = y[i] + dt * (1.0/5.0) * k1[i];
    QVector<double> k2 = func(t + dt / 5.0, y2);

    QVector<double> y3(n);
    for (int i = 0; i < n; ++i)
        y3[i] = y[i] + dt * (3.0/40.0 * k1[i] + 9.0/40.0 * k2[i]);
    QVector<double> k3 = func(t + 3.0 * dt / 10.0, y3);

    QVector<double> y4(n);
    for (int i = 0; i < n; ++i)
        y4[i] = y[i] + dt * (44.0/45.0 * k1[i] - 56.0/15.0 * k2[i] + 32.0/9.0 * k3[i]);
    QVector<double> k4 = func(t + 4.0 * dt / 5.0, y4);

    QVector<double> y5(n);
    for (int i = 0; i < n; ++i)
        y5[i] = y[i] + dt * (19372.0/6561.0 * k1[i] - 25360.0/2187.0 * k2[i]
                              + 64448.0/6561.0 * k3[i] - 212.0/729.0 * k4[i]);
    QVector<double> k5 = func(t + 8.0 * dt / 9.0, y5);

    QVector<double> y6(n);
    for (int i = 0; i < n; ++i)
        y6[i] = y[i] + dt * (9017.0/3168.0 * k1[i] - 355.0/33.0 * k2[i]
                              + 46732.0/5247.0 * k3[i] + 49.0/176.0 * k4[i]
                              - 5103.0/18656.0 * k5[i]);
    QVector<double> k6 = func(t + dt, y6);

    /* 5阶解 */
    QVector<double> result5(n);
    for (int i = 0; i < n; ++i)
        result5[i] = y[i] + dt * (35.0/384.0 * k1[i] + 500.0/1113.0 * k3[i]
                                   + 125.0/192.0 * k4[i] - 2187.0/6784.0 * k5[i]
                                   + 11.0/84.0 * k6[i]);

    /* 4阶解(误差估计用) */
    QVector<double> k7 = func(t + dt, result5);
    QVector<double> result4(n);
    for (int i = 0; i < n; ++i)
        result4[i] = y[i] + dt * (5179.0/57600.0 * k1[i] + 7571.0/16695.0 * k3[i]
                                   + 393.0/640.0 * k4[i] - 92097.0/339200.0 * k5[i]
                                   + 187.0/2100.0 * k6[i] + 1.0/40.0 * k7[i]);

    return {result5, result4};
}
