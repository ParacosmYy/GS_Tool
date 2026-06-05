/**
 * @file GaussLegendre.cpp
 * @brief Gauss-Legendre数值积分引擎实现 — 高精度数值积分
 */

#include "utils/gaussquad/GaussLegendre.h"

#include <QElapsedTimer>
#include <QPair>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
GaussLegendre::GaussLegendre(QObject* parent)
    : QObject(parent)
    , m_timeSumMs(0.0)
{
}

/**
 * @brief Gauss-Legendre数值积分
 * @param f 被积函数
 * @param a 积分下限
 * @param b 积分上限
 * @param n 求积点数(2~5)
 * @return 积分近似值
 *
 * 标准变换: x in [a,b] -> t in [-1,1]
 *   t = 2(x-a)/(b-a) - 1
 *   dx = (b-a)/2 * dt
 * 积分 = (b-a)/2 * sum(w_i * f(x_i))
 */
double GaussLegendre::integrate(std::function<double(double)> f,
                                double a, double b, int n)
{
    QElapsedTimer timer;
    timer.start();

    /* 限制n在有效范围内 */
    if (n < 2) n = 2;
    if (n > 5) n = 5;

    auto table = getNodesAndWeights(n);
    const QVector<double>& nodes = table.first;
    const QVector<double>& weights = table.second;

    double halfLen = (b - a) / 2.0;
    double mid = (a + b) / 2.0;

    double result = 0.0;
    for (int i = 0; i < nodes.size(); ++i) {
        /* 将[-1,1]上的节点映射回[a,b] */
        double x = mid + halfLen * nodes[i];
        result += weights[i] * f(x);
    }
    result *= halfLen;

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalIntegrations;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalIntegrations;

    emit integrationCompleted(result, n);
    return result;
}

/**
 * @brief 获取求积节点和权重
 * @param n 点数(2~5)
 * @return QPair(节点, 权重)
 *
 * 数据来源: Abramowitz & Stegun, Handbook of Mathematical Functions
 * 所有节点和权重均为精确值。
 */
QPair<QVector<double>, QVector<double>> GaussLegendre::getNodesAndWeights(int n)
{
    switch (n) {
    case 2: {
        static const double sqrtThird = 0.5773502691896257;
        return {
            {sqrtThird, -sqrtThird},
            {1.0, 1.0}
        };
    }
    case 3: {
        static const double s = 0.7745966692414834;
        return {
            {0.0, -s, s},
            {8.0 / 9.0, 5.0 / 9.0, 5.0 / 9.0}
        };
    }
    case 4: {
        static const double s1 = 0.3399810435848563;
        static const double s2 = 0.8611363115940526;
        return {
            {-s2, -s1, s1, s2},
            {0.3478548451374538, 0.6521451548625461,
             0.6521451548625461, 0.3478548451374538}
        };
    }
    case 5: {
        static const double s1 = 0.5384693101056831;
        static const double s2 = 0.9061798459386640;
        return {
            {0.0, -s1, s1, -s2, s2},
            {0.5688888888888889, 0.4786286704993665,
             0.4786286704993665, 0.2369268850561891,
             0.2369268850561891}
        };
    }
    default:
        return {{}, {}};
    }
}

/** @brief 重置统计信息 */
void GaussLegendre::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
