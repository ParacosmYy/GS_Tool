/**
 * @file PredictorCorrector.cpp
 * @brief 预估-校正法实现 — AB4-AM3 PECE格式
 */

#include "utils/predictor_corrector/PredictorCorrector.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
PredictorCorrector::PredictorCorrector(QObject* parent)
    : QObject(parent)
{
}

/** @brief RK4单步 */
double PredictorCorrector::rk4Step(OdeFunc f, double t, double y,
                                    double h) const
{
    double k1 = f(t, y);
    double k2 = f(t + h / 2.0, y + h * k1 / 2.0);
    double k3 = f(t + h / 2.0, y + h * k2 / 2.0);
    double k4 = f(t + h, y + h * k3);
    return y + h / 6.0 * (k1 + 2.0 * k2 + 2.0 * k3 + k4);
}

/** @brief 求解ODE */
QVector<QPair<double, double>> PredictorCorrector::solve(
    OdeFunc f, double y0, double t0, double tf, double h)
{
    QElapsedTimer timer;
    timer.start();

    QVector<QPair<double, double>> result;

    /* RK4启动: 计算前3个历史点(共4点用于AB4) */
    double t = t0;
    double y = y0;

    double tHist[4], yHist[4], fHist[4];
    tHist[0] = t;
    yHist[0] = y;
    fHist[0] = f(t, y);
    result.append({t, y});

    for (int i = 1; i < 4; ++i) {
        y = rk4Step(f, t, y, h);
        t = t0 + i * h;
        tHist[i] = t;
        yHist[i] = y;
        fHist[i] = f(t, y);
        result.append({t, y});
    }

    /* PECE迭代 */
    int step = 4;
    while (t < tf - h * 0.5) {
        /* P: AB4预估 */
        double yPred = yHist[3] + h / 24.0 * (
            55.0 * fHist[3] - 59.0 * fHist[2]
            + 37.0 * fHist[1] - 9.0 * fHist[0]);

        double tNew = t0 + step * h;

        /* E: 计算预估点的f */
        double fPred = f(tNew, yPred);

        /* C: AM3校正 */
        double yCorr = yHist[3] + h / 24.0 * (
            9.0 * fPred + 19.0 * fHist[3]
            - 5.0 * fHist[2] + fHist[1]);

        /* E: 计算校正点的f */
        double fCorr = f(tNew, yCorr);

        /* 更新历史(滑动窗口) */
        tHist[0] = tHist[1]; yHist[0] = yHist[1]; fHist[0] = fHist[1];
        tHist[1] = tHist[2]; yHist[1] = yHist[2]; fHist[1] = fHist[2];
        tHist[2] = tHist[3]; yHist[2] = yHist[3]; fHist[2] = fHist[3];
        tHist[3] = tNew;     yHist[3] = yCorr;    fHist[3] = fCorr;

        t = tNew;
        y = yCorr;
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
void PredictorCorrector::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
