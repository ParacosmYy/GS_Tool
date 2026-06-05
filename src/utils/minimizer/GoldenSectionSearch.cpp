/**
 * @file GoldenSectionSearch.cpp
 * @brief 黄金分割搜索实现
 */

#include "utils/minimizer/GoldenSectionSearch.h"

#include <QElapsedTimer>
#include <cmath>

/** @brief 黄金分割比常数 */
static constexpr double kGoldenRatio = 0.5 * (1.0 + std::sqrt(5.0));

/** @brief 构造函数 @param parent 父对象 */
GoldenSectionSearch::GoldenSectionSearch(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 在区间 [a,b] 上执行黄金分割搜索
 *
 * 每次迭代保留一个内点，仅需一次新函数求值，
 * 区间以 φ⁻¹ ≈ 0.618 的比率缩小直至收敛。
 */
double GoldenSectionSearch::minimize(Func f, double a, double b,
                                     double tol, int maxIter)
{
    QElapsedTimer timer;
    timer.start();

    /* 初始化两个内试探点 */
    double x1 = b - (b - a) / kGoldenRatio;
    double x2 = a + (b - a) / kGoldenRatio;
    double f1 = f(x1);
    double f2 = f(x2);

    int iter = 0;
    for (iter = 0; iter < maxIter; ++iter) {
        if (std::abs(b - a) < tol)
            break;

        if (f1 < f2) {
            /* 最小值在 [a, x2]，丢弃右半段 */
            b  = x2;
            x2 = x1;
            f2 = f1;
            x1 = b - (b - a) / kGoldenRatio;
            f1 = f(x1);
        } else {
            /* 最小值在 [x1, b]，丢弃左半段 */
            a  = x1;
            x1 = x2;
            f1 = f2;
            x2 = a + (b - a) / kGoldenRatio;
            f2 = f(x2);
        }
    }

    double minimum = (a + b) / 2.0;

    m_stats.totalMinimizations++;
    m_timeSum += timer.elapsed();
    m_stats.avgProcessingTimeMs = m_timeSum / m_stats.totalMinimizations;
    emit minimizationCompleted(minimum, iter);
    return minimum;
}

/** @brief 重置统计 */
void GoldenSectionSearch::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
