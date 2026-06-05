/**
 * @file LaguerrePoly.cpp
 * @brief Laguerre多项式实现 — 递推求值 + Newton-Laguerre求根
 */

#include "utils/poly4/LaguerrePoly.h"

#include <QElapsedTimer>

#include <algorithm>
#include <cmath>

/** @brief 构造函数 @param parent 父对象 */
LaguerrePoly::LaguerrePoly(QObject* parent)
    : QObject(parent)
{
}

/** @brief 三项递推系数 @param n 阶数 @param alpha 参数 @return 系数元组 */
std::tuple<double, double, double> LaguerrePoly::recurrenceCoeffs(int n, double alpha) const
{
    /* 广义Laguerre三项递推:
     * L_n^{alpha}(x) = ((2n - 1 + alpha - x) * L_{n-1}^{alpha}(x)
     *                  - (n - 1 + alpha) * L_{n-2}^{alpha}(x)) / n
     */
    if (n == 0) return {1.0, 0.0, 0.0};
    if (n == 1) return {1.0 + alpha, -1.0, 0.0};
    double a = (2.0 * n - 1.0 + alpha) / n;
    double b = -1.0 / n;
    double c = -(n - 1.0 + alpha) / n;
    return {a, b, c};
}

/** @brief 计算广义Laguerre多项式 @param n 阶数 @param alpha 参数 @param x 求值点 @return 值 */
double LaguerrePoly::evaluate(int n, double alpha, double x) const
{
    if (n < 0) return 0.0;
    if (n == 0) return 1.0;

    double prev2 = 1.0;                        /* L_0^{alpha}(x) = 1 */
    double prev1 = 1.0 + alpha - x;            /* L_1^{alpha}(x) = 1 + alpha - x */

    for (int k = 2; k <= n; ++k) {
        double curr = ((2.0 * k - 1.0 + alpha - x) * prev1
                       - (k - 1.0 + alpha) * prev2) / k;
        prev2 = prev1;
        prev1 = curr;
    }
    return prev1;
}

/** @brief 批量计算 @param n 最大阶数 @param alpha 参数 @param x 求值点 @return 值列表 */
std::vector<double> LaguerrePoly::evaluateAll(int n, double alpha, double x) const
{
    if (n < 0) return {};
    std::vector<double> values(n + 1);
    values[0] = 1.0;

    if (n >= 1) values[1] = 1.0 + alpha - x;

    for (int k = 2; k <= n; ++k) {
        values[k] = ((2.0 * k - 1.0 + alpha - x) * values[k - 1]
                      - (k - 1.0 + alpha) * values[k - 2]) / k;
    }
    return values;
}

/** @brief 计算导数 @param n 阶数 @param alpha 参数 @param x 求值点 @return 导数值 */
double LaguerrePoly::derivative(int n, double alpha, double x) const
{
    if (n <= 0) return 0.0;

    /* d/dx L_n^{alpha}(x) = -L_{n-1}^{alpha+1}(x) */
    return -evaluate(n - 1, alpha + 1.0, x);
}

/** @brief Laguerre方法求单个根 @param n 阶数 @param alpha 参数 @param initialGuess 初始猜测 @param maxIter 最大迭代 @return 根 */
double LaguerrePoly::laguerreRoot(int n, double alpha, double initialGuess, int maxIter) const
{
    double x = initialGuess;
    double tol = 1e-14;

    for (int iter = 0; iter < maxIter; ++iter) {
        double Lx = evaluate(n, alpha, x);
        double dLx = derivative(n, alpha, x);

        if (std::abs(Lx) < tol) return x;
        if (std::abs(dLx) < 1e-30) break;

        /* Newton步 */
        double step = Lx / dLx;

        /* 二阶导数用于自适应步长 */
        double d2Lx = 0.0;
        if (n >= 2) {
            /* d2/dx2 L_n^{alpha}(x) = L_{n-2}^{alpha+2}(x) */
            d2Lx = evaluate(n - 2, alpha + 2.0, x);
        }

        /* Halley方法修正 */
        double denom = 2.0 * dLx * dLx - Lx * d2Lx;
        if (std::abs(denom) > 1e-30) {
            step = 2.0 * Lx * dLx / denom;
        }

        double newX = x - step;
        if (newX < 0.0) newX = x / 2.0;
        if (std::abs(newX - x) < tol * std::max(1.0, std::abs(x))) return newX;
        x = newX;
    }

    return x;
}

/** @brief 求所有实根 @param n 阶数 @param alpha 参数 @return 根列表 */
std::vector<double> LaguerrePoly::findRoots(int n, double alpha)
{
    QElapsedTimer timer;
    timer.start();

    std::vector<double> roots;

    if (n <= 0) {
        m_timeSum += timer.elapsed();
        return roots;
    }

    /* 初始猜测: 使用Stroud-Secrest近似 */
    roots.reserve(n);

    /* 使用Gauss-Laguerre节点的近似公式 */
    for (int k = 1; k <= n; ++k) {
        /* 初始猜测: 近似节点位置 */
        double t = (n + 0.5 * (alpha + 1.0)) * (
            1.0 - std::cos(M_PI * (2.0 * k - 0.25) / (2.0 * n + 0.5 * (alpha + 1.0) + 0.5)));
        double guess = std::max(1e-6, t);

        double root = laguerreRoot(n, alpha, guess, 200);
        roots.push_back(root);
    }

    /* 排序去重 */
    std::sort(roots.begin(), roots.end());

    /* 去除重复根 */
    std::vector<double> uniqueRoots;
    uniqueRoots.reserve(roots.size());
    for (double r : roots) {
        if (uniqueRoots.empty() || std::abs(r - uniqueRoots.back()) > 1e-10) {
            uniqueRoots.push_back(r);
        }
    }

    /* Newton精修 */
    for (auto& root : uniqueRoots) {
        for (int iter = 0; iter < 50; ++iter) {
            double Lx = evaluate(n, alpha, root);
            double dLx = derivative(n, alpha, root);
            if (std::abs(dLx) < 1e-30) break;
            double step = Lx / dLx;
            double newRoot = root - step;
            if (newRoot < 0.0) newRoot = root / 2.0;
            if (std::abs(newRoot - root) < 1e-15) break;
            root = newRoot;
        }
    }

    m_stats.totalRootFindings++;
    m_stats.totalRootsFound += static_cast<int>(uniqueRoots.size());
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalEvaluations + m_stats.totalRootFindings + m_stats.totalIntegrations;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit rootsFound(n, static_cast<int>(uniqueRoots.size()));
    return uniqueRoots;
}

/** @brief 计算Gauss-Laguerre节点和权重 @param n 积分点数 @param alpha 参数 @return (节点, 权重) */
std::pair<std::vector<double>, std::vector<double>> LaguerrePoly::gaussNodesWeights(
    int n, double alpha)
{
    QElapsedTimer timer;
    timer.start();

    std::pair<std::vector<double>, std::vector<double>> result;
    if (n <= 0) return result;

    auto& nodes = result.first;
    auto& weights = result.second;

    nodes = findRoots(n, alpha);
    weights.resize(nodes.size());

    /* 权重公式: w_i = x_i^{alpha} * e^{-x_i} / (n! * [L_n'(x_i)]^2 / (n * L_{n-1}(x_i))) */
    /* 简化: w_i = Gamma(n+alpha+1) / (n! * x_i * [L_{n-1}^{alpha}(x_i)]^2) */
    double gammaNAlpha1 = 1.0;
    for (int i = 0; i < n; ++i) gammaNAlpha1 *= (n + alpha - i);

    for (size_t i = 0; i < nodes.size(); ++i) {
        double Lprev = evaluate(n - 1, alpha, nodes[i]);
        if (std::abs(Lprev) < 1e-30 || nodes[i] < 1e-30) {
            weights[i] = 0.0;
        } else {
            weights[i] = gammaNAlpha1 / (static_cast<double>(n) * nodes[i] * Lprev * Lprev);
        }
    }

    m_timeSum += timer.elapsed();
    return result;
}

/** @brief Gauss-Laguerre积分 @param n 积分点数 @param alpha 参数 @param f 被积函数 @return 积分值 */
double LaguerrePoly::integrate(int n, double alpha, const std::function<double(double)>& f)
{
    QElapsedTimer timer;
    timer.start();

    auto [nodes, weights] = gaussNodesWeights(n, alpha);

    double sum = 0.0;
    for (size_t i = 0; i < nodes.size(); ++i) {
        /* \int_0^inf f(x) * x^alpha * e^{-x} dx = \sum w_i * f(x_i) */
        sum += weights[i] * f(nodes[i]);
    }

    m_stats.totalIntegrations++;
    m_stats.totalEvaluations += n;
    m_timeSum += timer.elapsed();
    int totalOps = m_stats.totalEvaluations + m_stats.totalRootFindings + m_stats.totalIntegrations;
    m_stats.avgProcessingTimeMs = (totalOps > 0) ? m_timeSum / totalOps : 0.0;

    emit integrationCompleted(n, sum);
    return sum;
}

/** @brief 重置统计 */
void LaguerrePoly::resetStatistics()
{
    m_stats = Stats{};
    m_timeSum = 0.0;
}
