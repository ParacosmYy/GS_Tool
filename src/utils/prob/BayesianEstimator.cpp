/**
 * @file BayesianEstimator.cpp
 * @brief 贝叶斯参数估计器实现 — Beta共轭先验的伯努利推断
 */

#include "utils/prob/BayesianEstimator.h"

#include <QtMath>
#include <algorithm>

/** @brief 构造函数 @param parent 父对象 */
BayesianEstimator::BayesianEstimator(QObject* parent)
    : QObject(parent)
    , m_alpha(1.0)
    , m_beta(1.0)
    , m_totalSuccesses(0)
    , m_totalTrials(0)
    , m_timeSum(0.0)
{
}

/** @brief 设置Beta先验参数 @param alpha alpha参数 @param beta beta参数 */
void BayesianEstimator::setPrior(double alpha, double beta)
{
    m_alpha = qMax(0.01, alpha);
    m_beta  = qMax(0.01, beta);
    m_totalSuccesses = 0;
    m_totalTrials    = 0;
}

/** @brief 使用观测数据更新后验分布 @param successes 成功次数 @param trials 总试验次数 */
void BayesianEstimator::update(int successes, int trials)
{
    m_timer.start();

    int failures = trials - successes;
    if (failures < 0) failures = 0;
    if (successes < 0) successes = 0;

    /* Beta后验: alpha' = alpha + successes, beta' = beta + failures */
    m_alpha += static_cast<double>(successes);
    m_beta  += static_cast<double>(failures);
    m_totalSuccesses += successes;
    m_totalTrials    += trials;

    ++m_stats.totalUpdates;
    double elapsed = static_cast<double>(m_timer.elapsed());
    m_timeSum += elapsed;
    m_stats.avgProcessingTimeMs = m_timeSum
        / static_cast<double>(m_stats.totalUpdates);

    emit posteriorUpdated(posteriorMean());
}

/** @brief 计算后验均值 @return E[theta] = alpha / (alpha + beta) */
double BayesianEstimator::posteriorMean() const
{
    double sum = m_alpha + m_beta;
    if (qFuzzyIsNull(sum)) return 0.5;
    return m_alpha / sum;
}

/** @brief 计算后验方差 @return Var[theta] = ab / ((a+b)^2 * (a+b+1)) */
double BayesianEstimator::posteriorVariance() const
{
    double sum  = m_alpha + m_beta;
    if (sum <= 1.0) return 0.0;
    double numerator   = m_alpha * m_beta;
    double denominator = sum * sum * (sum + 1.0);
    if (qFuzzyIsNull(denominator)) return 0.0;
    return numerator / denominator;
}

/** @brief 计算可信区间 @param confidence 置信水平 @return QPair(下界, 上界) */
QPair<double, double> BayesianEstimator::credibleInterval(double confidence) const
{
    confidence = qBound(0.01, confidence, 0.999);
    double tail = (1.0 - confidence) / 2.0;

    /* 通过不完全Beta函数逆求区间端点 */
    double lower = betaInv(tail, m_alpha, m_beta);
    double upper = betaInv(1.0 - tail, m_alpha, m_beta);

    return qMakePair(qBound(0.0, lower, 1.0),
                     qBound(0.0, upper, 1.0));
}

/** @brief 预测下一次试验成功概率 @param newTrials 新试验次数 @return 预测概率 */
double BayesianEstimator::predict(int newTrials) const
{
    if (newTrials <= 0) newTrials = 1;

    /* 后验预测: P(X=k|data) 的期望值即为后验均值 */
    /* 对于二项预测, 期望成功数 = newTrials * posteriorMean */
    double sum = m_alpha + m_beta;
    if (qFuzzyIsNull(sum)) return 0.5;

    /* 拉普拉斯平滑后的预测概率 */
    double predictive = m_alpha / (m_alpha + m_beta + static_cast<double>(newTrials));
    /* 但连续情况下后验预测均值就是 posteriorMean */
    return m_alpha / sum;
}

/** @brief 重置统计 */
void BayesianEstimator::resetStatistics()
{
    m_stats  = Stats{};
    m_timeSum = 0.0;
}

/** @brief 不完全Beta函数 I_x(a,b) @param x 积分上限 @param a alpha @param b beta @return I_x(a,b) */
double BayesianEstimator::incompleteBeta(double x, double a, double b) const
{
    if (x <= 0.0) return 0.0;
    if (x >= 1.0) return 1.0;

    /* 利用对称性: I_x(a,b) = 1 - I_{1-x}(b,a) 提升收敛速度 */
    if (x > (a + 1.0) / (a + b + 2.0)) {
        return 1.0 - incompleteBeta(1.0 - x, b, a);
    }

    double lnPrefix = qLn(a + b) - qLn(a) - qLn(b)
                    + a * qLn(x) + b * qLn(1.0 - x);
    double prefix = qExp(lnPrefix);

    return prefix * betaCF(x, a, b);
}

/** @brief 连分数展开计算不完全Beta函数 @param x 积分上限 @param a alpha @param b beta @return 连分数值 */
double BayesianEstimator::betaCF(double x, double a, double b) const
{
    /* Lentz连分数算法 */
    double eps  = 1e-12;
    int maxIter = 200;

    double qab = a + b;
    double qap = a + 1.0;
    double qam = a - 1.0;

    double c = 1.0;
    double d = 1.0 - qab * x / qap;
    if (qFuzzyIsNull(d)) d = eps;
    d = 1.0 / d;
    double h = d;

    for (int m = 1; m <= maxIter; ++m) {
        int m2 = 2 * m;

        /* 偶数项系数 */
        double aa = static_cast<double>(m) * (b - static_cast<double>(m))
                  * x / ((qam + static_cast<double>(m2))
                  * (a + static_cast<double>(m2)));
        d = 1.0 + aa * d;
        if (qFuzzyIsNull(d)) d = eps;
        c = 1.0 + aa / c;
        if (qFuzzyIsNull(c)) c = eps;
        d = 1.0 / d;
        h *= d * c;

        /* 奇数项系数 */
        aa = -(a + static_cast<double>(m)) * (qab + static_cast<double>(m))
           * x / ((a + static_cast<double>(m2))
           * (qap + static_cast<double>(m2)));
        d = 1.0 + aa * d;
        if (qFuzzyIsNull(d)) d = eps;
        c = 1.0 + aa / c;
        if (qFuzzyIsNull(c)) c = eps;
        d = 1.0 / d;
        double del = d * c;
        h *= del;

        if (qAbs(del - 1.0) < eps) break;
    }

    return h;
}

/** @brief 二分法求不完全Beta函数的逆 @param target 目标值 @param a alpha @param b beta @return 满足I_x(a,b)=target的x */
double BayesianEstimator::betaInv(double target, double a, double b) const
{
    if (target <= 0.0) return 0.0;
    if (target >= 1.0) return 1.0;

    double lo = 0.0, hi = 1.0;
    double mid = 0.5;

    /* 二分法求解, 精度1e-8 */
    for (int i = 0; i < 60; ++i) {
        mid = (lo + hi) / 2.0;
        double val = incompleteBeta(mid, a, b);
        if (qAbs(val - target) < 1e-10) break;
        if (val < target) {
            lo = mid;
        } else {
            hi = mid;
        }
    }

    return mid;
}
