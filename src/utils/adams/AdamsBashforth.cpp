/**
 * @file AdamsBashforth.cpp
 * @brief Adams-Bashforth多步法实现
 */

#include "utils/adams/AdamsBashforth.h"

#include <QElapsedTimer>
#include <cmath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
AdamsBashforth::AdamsBashforth(QObject* parent)
    : QObject(parent)
{
}

/** @brief RK4单步 */
double AdamsBashforth::rk4Step(OdeFunc f, double t, double y, double h) const
{
    double k1 = f(t, y);
    double k2 = f(t + h / 2.0, y + h * k1 / 2.0);
    double k3 = f(t + h / 2.0, y + h * k2 / 2.0);
    double k4 = f(t + h, y + h * k3);
    return y + h / 6.0 * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

/** @brief 求解ODE */
QVector<QPair<double, double>> AdamsBashforth::solve(
    OdeFunc f, double y0, double t0, double tf, double h, int order)
{
    QElapsedTimer timer;
    timer.start();

    order = std::clamp(order, 1, 4);

    QVector<QPair<double, double>> result;

    /* 用RK4启动，计算前order-1个历史点 */
    QVector<double> tHist, yHist, fHist;

    double t = t0;
    double y = y0;
    tHist.append(t);
    yHist.append(y);
    fHist.append(f(t, y));
    result.append({t, y});

    for (int i = 1; i < order; ++i) {
        y = rk4Step(f, t, y, h);
        t = t0 + i * h;
        tHist.append(t);
        yHist.append(y);
        fHist.append(f(t, y));
        result.append({t, y});
    }

    /* AB多步迭代 */
    int step = order;
    while (t < tf - h * 0.5) {
        double fNew = 0.0;

        if (order == 1) {
            /* AB1 = 前向Euler */
            fNew = fHist.last();
        } else if (order == 2) {
            /* AB2 */
            fNew = 1.5 * fHist[1] - 0.5 * fHist[0];
        } else if (order == 3) {
            /* AB3 */
            fNew = (23.0 * fHist[2] - 16.0 * fHist[1]
                    + 5.0 * fHist[0]) / 12.0;
        } else {
            /* AB4 */
            fNew = (55.0 * fHist[3] - 59.0 * fHist[2]
                    + 37.0 * fHist[1] - 9.0 * fHist[0]) / 24.0;
        }

        y = yHist.last() + h * fNew;
        t = t0 + step * h;

        /* 更新历史 */
        tHist.append(t);
        yHist.append(y);
        fHist.append(f(t, y));

        /* 只保留最近order个历史 */
        while (tHist.size() > order) {
            tHist.removeFirst();
            yHist.removeFirst();
            fHist.removeFirst();
        }

        result.append({t, y});
        step++;
    }

    m_stats.totalSolves++;
    m_stats.totalSteps += step;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalSolves;

    emit solveCompleted(step, t);
    return result;
}

/** @brief 重置统计 */
void AdamsBashforth::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
