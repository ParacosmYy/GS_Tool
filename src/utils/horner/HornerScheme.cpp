/**
 * @file HornerScheme.cpp
 * @brief Horner多项式求值引擎实现 — 高效多项式计算与导数
 */

#include "utils/horner/HornerScheme.h"

#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
HornerScheme::HornerScheme(QObject* parent)
    : QObject(parent)
    , m_timeSumMs(0.0)
{
}

/**
 * @brief 使用Horner法则计算多项式值
 * @param coeffs 多项式系数(从高次到低次)
 * @param x 求值点
 * @return 多项式在x处的值
 *
 * Horner法则: P(x) = (...((a0*x + a1)*x + a2)*x + ... + an)
 * 乘法次数从O(n²)降为O(n)，且数值稳定性更好。
 */
double HornerScheme::evaluate(const QVector<double>& coeffs, double x)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.isEmpty()) {
        ++m_stats.totalEvaluations;
        emit evaluationCompleted(0.0);
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
    ++m_stats.totalEvaluations;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalEvaluations;

    emit evaluationCompleted(result);
    return result;
}

/**
 * @brief 同时计算多项式值和一阶导数
 * @param coeffs 多项式系数(从高次到低次)
 * @param x 求值点
 * @return QPair(多项式值, 导数值)
 *
 * 对P(x)使用Horner法则求值的同时，对降次后的多项式再次使用
 * Horner法则得到P'(x)。额外开销极小。
 */
QPair<double, double> HornerScheme::evaluateWithDerivative(
    const QVector<double>& coeffs, double x)
{
    QElapsedTimer timer;
    timer.start();

    if (coeffs.isEmpty()) {
        ++m_stats.totalEvaluations;
        emit evaluationCompleted(0.0);
        return {0.0, 0.0};
    }
    if (coeffs.size() == 1) {
        ++m_stats.totalEvaluations;
        emit evaluationCompleted(coeffs[0]);
        return {coeffs[0], 0.0};
    }

    /* 第一步Horner: P(x)值，同时降次得到导数多项式的系数 */
    double polyValue = coeffs[0];
    double derivValue = 0.0;

    for (int i = 1; i < coeffs.size(); ++i) {
        derivValue = derivValue * x + polyValue;
        polyValue = polyValue * x + coeffs[i];
    }

    /* 更新统计 */
    double elapsed = timer.nsecsElapsed() / 1e6;
    m_timeSumMs += elapsed;
    ++m_stats.totalEvaluations;
    m_stats.avgProcessingTimeMs = m_timeSumMs / m_stats.totalEvaluations;

    emit evaluationCompleted(polyValue);
    return {polyValue, derivValue};
}

/** @brief 重置统计信息 */
void HornerScheme::resetStatistics()
{
    m_stats = Stats{};
    m_timeSumMs = 0.0;
}
