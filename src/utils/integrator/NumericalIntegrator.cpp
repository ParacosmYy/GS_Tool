/**
 * @file NumericalIntegrator.cpp
 * @brief 数值积分引擎实现 — 矩形/梯形/辛普森/龙贝格积分
 */

#include "utils/integrator/NumericalIntegrator.h"

#include <QtMath>
#include <QElapsedTimer>

NumericalIntegrator::NumericalIntegrator(QObject* parent)
    : QObject(parent), m_method(Method::Trapezoidal), m_dx(1.0), m_valueSum(0.0) {}

void NumericalIntegrator::setMethod(Method method) { m_method = method; }
void NumericalIntegrator::setDx(double dx) { m_dx = (dx > 0) ? dx : 1.0; }

/** @brief 定积分 @param data 数据 @return 积分值 */
double NumericalIntegrator::integrate(const QVector<double>& data)
{
    if (data.size() < 2) return 0.0;

    double result = 0.0;
    switch (m_method) {
    case Method::Rectangle:    result = integrateRectangle(data); break;
    case Method::Trapezoidal:  result = integrateTrapezoidal(data); break;
    case Method::Simpson:      result = integrateSimpson(data); break;
    case Method::Romberg:      result = integrateRomberg(data); break;
    }

    ++m_stats.totalIntegrations;
    m_stats.totalPointsProcessed += static_cast<quint64>(data.size());
    m_valueSum += qAbs(result);
    if (qAbs(result) > m_stats.peakValue) m_stats.peakValue = qAbs(result);
    m_stats.averageValue = m_valueSum / m_stats.totalIntegrations;

    emit integrationComplete(result);
    return result;
}

/** @brief 累计积分 @param data 数据 @return 累计积分曲线 */
QVector<double> NumericalIntegrator::cumulativeIntegrate(const QVector<double>& data)
{
    if (data.size() < 2) return {};

    QVector<double> result;
    result.reserve(data.size());
    double sum = 0.0;
    result.append(0.0);

    for (int i = 1; i < data.size(); ++i) {
        sum += (data[i - 1] + data[i]) / 2.0 * m_dx;
        result.append(sum);
    }
    return result;
}

/** @brief 区间积分 @param data 数据 @param start 起始 @param end 结束 @return 积分值 */
double NumericalIntegrator::integrateRange(const QVector<double>& data,
                                            int start, int end)
{
    if (start < 0) start = 0;
    if (end >= data.size()) end = data.size() - 1;
    if (start >= end) return 0.0;

    QVector<double> slice(data.constBegin() + start, data.constBegin() + end + 1);
    return integrate(slice);
}

void NumericalIntegrator::resetStatistics()
{
    m_stats = Stats{};
    m_valueSum = 0.0;
}

/** @brief 矩形法 @param data 数据 @return 积分值 */
double NumericalIntegrator::integrateRectangle(const QVector<double>& data) const
{
    double sum = 0.0;
    for (int i = 0; i < data.size() - 1; ++i) {
        sum += data[i] * m_dx;
    }
    return sum;
}

/** @brief 梯形法 @param data 数据 @return 积分值 */
double NumericalIntegrator::integrateTrapezoidal(const QVector<double>& data) const
{
    double sum = 0.0;
    for (int i = 0; i < data.size() - 1; ++i) {
        sum += (data[i] + data[i + 1]) / 2.0 * m_dx;
    }
    return sum;
}

/** @brief 辛普森法 @param data 数据 @return 积分值 */
double NumericalIntegrator::integrateSimpson(const QVector<double>& data) const
{
    int n = data.size() - 1;
    if (n < 2) return integrateTrapezoidal(data);
    if (n % 2 != 0) {
        /* 奇数段: 最后一小段用梯形法 */
        double trap = (data[n - 1] + data[n]) / 2.0 * m_dx;
        QVector<double> sub(data.constBegin(), data.constBegin() + n);
        return integrateSimpson(sub) + trap;
    }

    double sum = data[0] + data[n];
    for (int i = 1; i < n; i += 2) {
        sum += 4.0 * data[i];
    }
    for (int i = 2; i < n; i += 2) {
        sum += 2.0 * data[i];
    }
    return sum * m_dx / 3.0;
}

/** @brief 龙贝格外推 @param data 数据 @return 积分值 */
double NumericalIntegrator::integrateRomberg(const QVector<double>& data) const
{
    /* 以梯形法为基础，外推2步 */
    double T1 = integrateTrapezoidal(data);

    /* 简化: 使用复合梯形n=2和n=4外推 */
    int n = data.size();
    if (n < 5) return T1;

    double h = (n - 1) * m_dx;
    double T2 = 0.0, T4 = 0.0;
    int step2 = qMax(1, (n - 1) / 2);
    int step4 = qMax(1, (n - 1) / 4);

    for (int i = 0; i < n - 1; i += step2) {
        int j = qMin(i + step2, n - 1);
        T2 += (data[i] + data[j]) / 2.0 * (step2 * m_dx);
    }
    for (int i = 0; i < n - 1; i += step4) {
        int j = qMin(i + step4, n - 1);
        T4 += (data[i] + data[j]) / 2.0 * (step4 * m_dx);
    }

    /* Richardson外推 */
    double S1 = T2 + (T2 - T1) / 3.0;
    double S2 = T4 + (T4 - T2) / 3.0;
    return S2 + (S2 - S1) / 15.0;
}
