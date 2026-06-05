/**
 * @file RichardsonExtrapolation.cpp
 * @brief Richardson外推引擎实现 — 数值精度提升方法
 */

#include "utils/richardson/RichardsonExtrapolation.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
RichardsonExtrapolation::RichardsonExtrapolation(QObject* parent)
    : QObject(parent)
    , m_timeSumMs(0.0)
{
}

/**
 * @brief 对多个近似值进行Richardson外推
 * @param approximations 不同步长下的近似值
 * @param steps 对应步长
 * @return 外推后的高精度结果
 *
 * 使用Neville表格递推:
 *   T[i][j] = T[i][j-1] + (T[i][j-1] - T[i-1][j-1]) /
 *             ((h[i-j]/h[i])^order - 1)
 * 假设误差阶数为2(O(h²))，这是最常见的场景。
 * 对于一般情况，使用逐列消元消除最低阶误差项。
 */
double RichardsonExtrapolation::extrapolate(
    const QVector<double>& approximations,
    const QVector<double>& steps)
{
    QElapsedTimer timer;
    timer.start();

    int n = qMin(approximations.size(), steps.size());
    double result = 0.0;

    if (n == 0) {
        /* 无数据，返回0 */
    } else if (n == 1) {
        result = approximations[0];
    } else {
        /* 构建Neville外推表格 */
        QVector<double> table(approximations);

        for (int j = 1; j < n; ++j) {
            for (int i = n - 1; i >= j; --i) {
                double ratio = steps[i - j] / steps[i];
                double factor = ratio * ratio - 1.0;

                if (std::abs(factor) < 1e-15) {
                    /* 避免除零 */
                    continue;
                }
                table[i] = table[i] +
                    (table[i] - table[i - 1]) / factor;
            }
        }
        result = table[n - 1];
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalExtrapolations;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalExtrapolations;

    emit extrapolationCompleted(result);
    return result;
}

/**
 * @brief 对函数值进行步长细化和外推
 * @param f 目标函数
 * @param x 求值点
 * @param h 初始步长
 * @param order 误差阶数
 * @return 外推后的高精度值
 *
 * 使用4个递减步长(h, h/2, h/4, h/8)计算近似值，
 * 然后用Neville表格进行多级外推消除O(h^order)误差项。
 */
double RichardsonExtrapolation::refine(
    std::function<double(double)> f,
    double x, double h, int order)
{
    QElapsedTimer timer;
    timer.start();

    /* 生成4个递减步长的近似值 */
    const int numSamples = 4;
    QVector<double> approximations(numSamples);
    QVector<double> steps(numSamples);

    double currentH = h;
    for (int i = 0; i < numSamples; ++i) {
        steps[i] = currentH;
        approximations[i] = f(x + currentH);
        currentH /= 2.0;
    }

    /* 构建Neville外推表格，使用指定的误差阶数 */
    QVector<double> table(approximations);

    for (int j = 1; j < numSamples; ++j) {
        for (int i = numSamples - 1; i >= j; --i) {
            double ratio = steps[i - j] / steps[i];
            double power = 1.0;
            for (int p = 0; p < order; ++p) {
                power *= ratio;
            }
            double factor = power - 1.0;

            if (std::abs(factor) < 1e-15) {
                continue;
            }
            table[i] = table[i] +
                (table[i] - table[i - 1]) / factor;
        }
    }
    double result = table[numSamples - 1];

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalExtrapolations;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalExtrapolations;

    emit extrapolationCompleted(result);
    return result;
}

/** @brief 重置统计信息 */
void RichardsonExtrapolation::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
