/**
 * @file HornerScheme.cpp
 * @brief Horner法则增强版实现 — 多项式求值/导数/降次
 */

#include "utils/horner2/HornerScheme.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
HornerEval::HornerEval(QObject* parent)
    : QObject(parent)
{
}

/**
 * @brief 使用Horner法则计算多项式值
 * @param coeffs 多项式系数(从高次到低次)
 * @param x 求值点
 * @return 多项式在x处的值
 *
 * Horner法则: P(x) = (...((a0*x + a1)*x + a2)*x + ... + an)
 * 乘法次数从O(n^2)降为O(n)，且数值稳定性更好。
 */
double HornerEval::evaluate(const QVector<double>& coeffs, double x)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.isEmpty()) {
        ++m_stats.totalEvaluated;
        m_stats.avgProcessingTimeMs = (m_stats.totalEvaluated > 0)
            ? m_timeSumMs / m_stats.totalEvaluated : 0.0;
        emit evaluated(0);
        return 0.0;
    }

    /* Horner法则: 从最高次系数开始递推 */
    double result = coeffs[0];
    for (int i = 1; i < coeffs.size(); ++i) {
        result = result * x + coeffs[i];
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalEvaluated;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalEvaluated;

    emit evaluated(coeffs.size() - 1);
    return result;
}

/**
 * @brief 计算一阶导数在x处的值
 * @param coeffs 多项式系数(从高次到低次)
 * @param x 求值点
 * @return 导数值
 *
 * P'(x) = n*a0*x^(n-1) + (n-1)*a1*x^(n-2) + ... + a(n-1)
 * 使用Horner法则: 导数多项式为原多项式降次后的系数。
 */
double HornerEval::derivative(const QVector<double>& coeffs, double x)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.size() <= 1) {
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalEvaluated;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalEvaluated;
        emit evaluated(coeffs.size() - 1);
        return 0.0;
    }

    /* Horner法则求导: 对降次后的多项式求值 */
    int degree = coeffs.size() - 1;
    double result = coeffs[0] * degree;
    for (int i = 1; i < degree; ++i) {
        result = result * x + coeffs[i] * (degree - i);
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalEvaluated;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalEvaluated;

    emit evaluated(coeffs.size() - 1);
    return result;
}

/**
 * @brief 计算所有阶导数在x处的值
 * @param coeffs 多项式系数(从高次到低次)
 * @param x 求值点
 * @return 各阶导数值 [P(x), P'(x), P''(x), ...]
 *
 * 使用嵌套Horner法则，一次遍历计算所有阶导数。
 * b[0]为多项式值，b[k]为k阶导数/k!的值(需乘以k!)。
 */
QVector<double> HornerEval::allDerivatives(const QVector<double>& coeffs,
                                              double x)
{
    QElapsedTimer timer;
    timer.start();

    int n = coeffs.size();
    if (n == 0) {
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalEvaluated;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalEvaluated;
        emit evaluated(0);
        return {};
    }

    /* 初始化导数表 */
    QVector<double> derivs(n, 0.0);

    /* 嵌套Horner: 同时计算P(x)及各阶导数 */
    derivs[0] = coeffs[0];
    for (int i = 1; i < n; ++i) {
        /* 从高阶向低阶更新导数值 */
        for (int j = i; j >= 1; --j) {
            derivs[j] = derivs[j] * x + derivs[j - 1];
        }
        derivs[0] = derivs[0] * x + coeffs[i];
    }

    /* derivs[k]存储的是第k阶导数/k!，需要乘以k!恢复 */
    double factorial = 1.0;
    for (int k = 1; k < n; ++k) {
        factorial *= static_cast<double>(k);
        derivs[k] *= factorial;
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalEvaluated;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalEvaluated;

    emit evaluated(n - 1);
    return derivs;
}

/**
 * @brief 综合除法 — 用已知根降次
 * @param coeffs 原多项式系数(从高次到低次)
 * @param root 已知根
 * @return 降次后的多项式系数(次数减1)
 *
 * P(x) = (x - root) * Q(x)，Q(x)为降次后的多项式。
 * 使用Horner法则高效计算商多项式的系数。
 */
QVector<double> HornerEval::deflate(const QVector<double>& coeffs,
                                       double root)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.size() <= 1) {
        m_timeSumMs += timer.nsecsElapsed() / 1e6;
        ++m_stats.totalEvaluated;
        m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalEvaluated;
        emit evaluated(0);
        return {};
    }

    int n = coeffs.size();
    QVector<double> quotient(n - 1, 0.0);

    /* 综合除法: Horner递推 */
    quotient[0] = coeffs[0];
    for (int i = 1; i < n - 1; ++i) {
        quotient[i] = quotient[i - 1] * root + coeffs[i];
    }

    /* 余数 = quotient[n-2]*root + coeffs[n-1] (应为接近0) */

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalEvaluated;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalEvaluated;

    emit evaluated(n - 2);
    return quotient;
}

/** @brief 重置统计信息 */
void HornerEval::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
