/**
 * @file LobattoIntegration.cpp
 * @brief Gauss-Lobatto求积实现
 */

#include "utils/lobatto/LobattoIntegration.h"

#include <QElapsedTimer>
#include <QtMath>

#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
LobattoIntegration::LobattoIntegration(QObject* parent)
    : QObject(parent)
    , m_timeSum(0.0)
{
}

/** @brief 执行Gauss-Lobatto数值积分 */
double LobattoIntegration::integrate(
    const std::function<double(double)>& f,
    double a, double b, int n)
{
    QElapsedTimer timer;
    timer.start();

    if (n < 2) n = 2;

    /* 计算节点和权重 */
    QVector<double> nodes;
    QVector<double> weights;
    computeNodesAndWeights(n, nodes, weights);

    /* 从[-1,1]映射到[a,b]: x = (b-a)/2 * t + (b+a)/2 */
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

/** @brief 计算Lobatto节点和权重 */
void LobattoIntegration::computeNodesAndWeights(
    int n, QVector<double>& nodes, QVector<double>& weights)
{
    nodes.resize(n);
    weights.resize(n);

    if (n == 2) {
        /* 2点Lobatto: 端点-1和1 */
        nodes[0] = -1.0;
        nodes[1] =  1.0;
        weights[0] = 1.0;
        weights[1] = 1.0;
        return;
    }

    if (n == 3) {
        /* 3点Lobatto */
        nodes[0] = -1.0;
        nodes[1] =  0.0;
        nodes[2] =  1.0;
        weights[0] = 1.0 / 3.0;
        weights[1] = 4.0 / 3.0;
        weights[2] = 1.0 / 3.0;
        return;
    }

    /* 端点 */
    nodes[0] = -1.0;
    nodes[n - 1] = 1.0;

    /* 内部节点: P'_{n-1}(x)的根，用Newton迭代 */
    int numInterior = n - 2;
    for (int k = 1; k <= numInterior; ++k) {
        /* 初始猜测: Chebyshev节点偏移 */
        double x = -qCos(static_cast<double>(k) * M_PI
                         / static_cast<double>(n - 1));

        /* Newton迭代求P'_{n-1}(x)=0 */
        for (int iter = 0; iter < 100; ++iter) {
            double pn = legendreP(n - 1, x);
            double dpn = legendrePderivative(n - 1, x);
            if (qAbs(dpn) < 1e-30) break;

            /* P'_{n-1}的导数需要P_{n-1}和P_{n-2} */
            double pn1 = legendreP(n, x);
            /* P'_{n}的导数(用于二阶Newton修正) */
            double correction = pn * pn1
                / (static_cast<double>(n) * (dpn * dpn - pn * pn1));

            x -= pn / dpn;

            if (qAbs(pn / dpn) < 1e-15) break;
        }

        nodes[k] = x;
    }

    /* 权重: w_i = 2 / (n * (n-1) * [P_{n-1}(x_i)]^2) */
    double factor = 2.0 / static_cast<double>(n * (n - 1));
    for (int i = 0; i < n; ++i) {
        double pi = legendreP(n - 1, nodes[i]);
        weights[i] = factor / (pi * pi);
    }
}

/** @brief Legendre多项式P_n(x) */
double LobattoIntegration::legendreP(int n, double x)
{
    if (n == 0) return 1.0;
    if (n == 1) return x;

    double pPrev2 = 1.0;
    double pPrev1 = x;
    double pCurr = 0.0;

    for (int k = 2; k <= n; ++k) {
        pCurr = (static_cast<double>(2 * k - 1) * x * pPrev1
                 - static_cast<double>(k - 1) * pPrev2)
                / static_cast<double>(k);
        pPrev2 = pPrev1;
        pPrev1 = pCurr;
    }
    return pCurr;
}

/** @brief Legendre多项式导数P'_n(x) */
double LobattoIntegration::legendrePderivative(int n, double x)
{
    if (n == 0) return 0.0;
    if (n == 1) return 1.0;

    /* P'_n(x) = n * (x * P_n(x) - P_{n-1}(x)) / (x^2 - 1) */
    double pn = legendreP(n, x);
    double pn1 = legendreP(n - 1, x);
    double denom = x * x - 1.0;

    if (qAbs(denom) < 1e-15) {
        /* 在端点±1处使用极限: P'_n(1) = n*(n+1)/2 */
        double sign = (x > 0) ? 1.0 : ((x < 0) ? -1.0 : 0.0);
        /* P'_n(1) = n*(n+1)/2, P'_n(-1) = (-1)^{n+1} * n*(n+1)/2 */
        double val = static_cast<double>(n * (n + 1)) / 2.0;
        if (sign < 0) {
            val *= (n % 2 == 0) ? -1.0 : 1.0;
        }
        return val;
    }

    return static_cast<double>(n) * (x * pn - pn1) / denom;
}

/** @brief 重置统计 */
void LobattoIntegration::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
