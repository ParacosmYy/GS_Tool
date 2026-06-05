/**
 * @file ClenshawCurtis.cpp
 * @brief Clenshaw-Curtis求积实现
 */

#include "utils/clenshaw_curtis/ClenshawCurtis.h"

#include <QElapsedTimer>
#include <QtMath>

#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
ClenshawCurtis::ClenshawCurtis(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 执行Clenshaw-Curtis数值积分 */
double ClenshawCurtis::integrate(
    const std::function<double(double)>& f,
    double a, double b, int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n < 2) n = 2;

    /* 计算Chebyshev节点和权重 */
    QVector<double> nodes = computeNodes(n);
    QVector<double> weights = computeWeights(n);

    /* 从[-1,1]映射到[a,b] */
    double halfLen = (b - a) / 2.0;
    double mid = (b + a) / 2.0;

    double result = 0.0;
    for (int i = 0; i < n; ++i) {
        double x = halfLen * nodes[i] + mid;
        result += weights[i] * f(x);
    }
    result *= halfLen;

    /* 更新统计 */
    double elapsed = static_cast<double>(timer.elapsed());
    m_timeSum += elapsed;
    ++m_stats.totalIntegrations;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalIntegrations);

    emit integrationCompleted(result, n);
    return result;
}

/** @brief 计算Chebyshev节点
 *  x_k = cos(k * pi / (n-1))，k=0..n-1 */
QVector<double> ClenshawCurtis::computeNodes(int n)
{
    QVector<double> nodes(n);
    if (n == 1) {
        nodes[0] = 0.0;
        return nodes;
    }

    for (int k = 0; k < n; ++k) {
        nodes[k] = qCos(static_cast<double>(k) * M_PI
                        / static_cast<double>(n - 1));
    }
    return nodes;
}

/** @brief 计算Clenshaw-Curtis权重(通过DCT-II) */
QVector<double> ClenshawCurtis::computeWeights(int n)
{
    QVector<double> w(n, 0.0);
    if (n == 1) {
        w[0] = 2.0;
        return w;
    }

    int N = n - 1;

    /* 计算 g_k = 2 / (1 - 4*k^2)，k=0..N */
    QVector<double> g(N + 1);
    for (int k = 0; k <= N; ++k) {
        double denom = 1.0 - 4.0 * static_cast<double>(k * k);
        if (qAbs(denom) < 1e-30) {
            g[k] = 1.0;  ///< k=0处特殊处理
        } else {
            g[k] = 2.0 / denom;
        }
    }

    /* 通过DCT-I计算权重
     * w_j = (2/N) * sum_{k=0}^{N} g_k * cos(k*j*pi/N) * c_k
     * c_0 = c_N = 0.5, c_k = 1 (其他) */
    for (int j = 0; j < n; ++j) {
        double sum = 0.0;
        double cj = 1.0;
        if (j == 0 || j == N) cj = 0.5;
        for (int k = 0; k <= N; ++k) {
            double ck = 1.0;
            if (k == 0 || k == N) ck = 0.5;

            sum += ck * g[k] * qCos(static_cast<double>(k * j) * M_PI
                                    / static_cast<double>(N));
        }
        w[j] = (cj * sum) / static_cast<double>(N);
    }

    return w;
}

/** @brief 重置统计 */
void ClenshawCurtis::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
