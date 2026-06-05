/**
 * @file RombergIntegration.cpp
 * @brief Romberg数值积分实现
 */

#include "utils/romberg/RombergIntegration.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
RombergIntegration::RombergIntegration(QObject* parent)
    : QObject(parent)
{
}

/** @brief Romberg积分 */
double RombergIntegration::integrate(Func f, double a, double b,
                                      int maxLevels)
{
    QElapsedTimer timer;
    timer.start();

    if (maxLevels <= 0) maxLevels = 1;

    /* R表: R[k][j] */
    QVector<QVector<double>> R(maxLevels);

    /* R[0][0] = 梯形法1段 */
    double h = b - a;
    R[0].resize(1);
    R[0][0] = h / 2.0 * (f(a) + f(b));

    int usedLevels = 1;

    for (int k = 1; k < maxLevels; ++k) {
        h /= 2.0;
        int n = 1 << k; // 2^k

        /* 新增点求和 */
        double sum = 0.0;
        for (int i = 1; i < n; i += 2)
            sum += f(a + i * h);

        /* R[k][0] = 梯形法 */
        R[k].resize(k + 1);
        R[k][0] = R[k - 1][0] / 2.0 + h * sum;

        /* Richardson外推 */
        double fourJ = 1.0;
        for (int j = 1; j <= k; ++j) {
            fourJ *= 4.0;
            R[k][j] = (fourJ * R[k][j - 1] - R[k - 1][j - 1])
                       / (fourJ - 1.0);
        }

        usedLevels = k + 1;

        /* 收敛判断: 比较对角线元素 */
        if (k >= 2) {
            double diff = std::abs(R[k][k] - R[k - 1][k - 1]);
            if (diff < 1e-14 * std::abs(R[k][k]) + 1e-300)
                break;
        }
    }

    double result = R[usedLevels - 1][usedLevels - 1];

    m_stats.totalIntegrations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalIntegrations;

    emit integrationCompleted(result, usedLevels);
    return result;
}

/** @brief 重置统计 */
void RombergIntegration::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
