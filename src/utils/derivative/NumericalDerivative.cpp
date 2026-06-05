/**
 * @file NumericalDerivative.cpp
 * @brief 数值微分引擎实现 — 前向/后向/中心差分/SG滤波微分
 */

#include "utils/derivative/NumericalDerivative.h"

#include <QtMath>
#include <QElapsedTimer>

/** @brief 构造函数 @param parent 父对象 */
NumericalDerivative::NumericalDerivative(QObject* parent)
    : QObject(parent)
    , m_method(DiffMethod::Central)
    , m_dx(1.0)
    , m_derivSum(0.0)
{
}

/** @brief 设置差分方法 @param method 方法 */
void NumericalDerivative::setMethod(DiffMethod method)
{
    m_method = method;
}

/** @brief 设置采样间隔 @param dx 间隔 */
void NumericalDerivative::setDx(double dx)
{
    m_dx = (dx > 0) ? dx : 1.0;
}

/** @brief 一阶微分 @param data 数据 @return 导数 */
QVector<double> NumericalDerivative::derivative(const QVector<double>& data)
{
    if (data.size() < 2) return {};

    QVector<double> result;
    switch (m_method) {
    case DiffMethod::Forward:
        result = diffForward(data);
        break;
    case DiffMethod::Backward:
        result = diffBackward(data);
        break;
    case DiffMethod::Central:
        result = diffCentral(data);
        break;
    case DiffMethod::SavitzkyGolay:
        result = diffSavitzkyGolay(data);
        break;
    }

    ++m_stats.totalDerivatives;
    m_stats.totalPointsProcessed += static_cast<quint64>(data.size());

    double sumAbs = 0.0;
    for (double v : result) {
        sumAbs += qAbs(v);
        if (qAbs(v) > m_stats.peakDerivative) {
            m_stats.peakDerivative = qAbs(v);
        }
    }
    m_derivSum += sumAbs;
    m_stats.averageDerivative = m_derivSum
        / static_cast<double>(m_stats.totalPointsProcessed);

    emit derivativeComputed(1, result.size());
    return result;
}

/** @brief 二阶微分 @param data 数据 @return 二阶导数 */
QVector<double> NumericalDerivative::secondDerivative(const QVector<double>& data)
{
    /* 先求一阶，再对一阶求导 */
    QVector<double> first = derivative(data);
    if (first.isEmpty()) return {};

    QVector<double> result;
    result.reserve(first.size());
    double dx2 = m_dx * m_dx;

    for (int i = 1; i < first.size() - 1; ++i) {
        result.append((first[i + 1] - 2.0 * first[i] + first[i - 1]) / (dx2));
    }
    /* 边界: 复制相邻值 */
    if (!result.isEmpty()) {
        result.prepend(result.first());
        result.append(result.last());
    }

    emit derivativeComputed(2, result.size());
    return result;
}

/** @brief N阶微分 @param data 数据 @param order 阶数 @return N阶导数 */
QVector<double> NumericalDerivative::derivativeN(const QVector<double>& data,
                                                   int order)
{
    if (order < 1 || data.size() < order + 1) return {};

    QVector<double> current = data;
    for (int o = 0; o < order; ++o) {
        current = derivative(current);
        if (current.isEmpty()) return {};
    }

    emit derivativeComputed(order, current.size());
    return current;
}

/** @brief 重置统计 */
void NumericalDerivative::resetStatistics()
{
    m_stats = Stats{};
    m_derivSum = 0.0;
}

/** @brief 前向差分 @param data 数据 @return 导数 */
QVector<double> NumericalDerivative::diffForward(
    const QVector<double>& data) const
{
    QVector<double> result;
    result.reserve(data.size());
    int n = data.size();

    for (int i = 0; i < n - 1; ++i) {
        result.append((data[i + 1] - data[i]) / m_dx);
    }
    if (!result.isEmpty()) {
        result.append(result.last());
    }
    return result;
}

/** @brief 后向差分 @param data 数据 @return 导数 */
QVector<double> NumericalDerivative::diffBackward(
    const QVector<double>& data) const
{
    QVector<double> result;
    result.reserve(data.size());

    result.append(0.0);
    for (int i = 1; i < data.size(); ++i) {
        result.append((data[i] - data[i - 1]) / m_dx);
    }
    if (result.size() > 1) {
        result[0] = result[1];
    }
    return result;
}

/** @brief 中心差分 @param data 数据 @return 导数 */
QVector<double> NumericalDerivative::diffCentral(
    const QVector<double>& data) const
{
    QVector<double> result;
    result.reserve(data.size());
    int n = data.size();

    /* 边界用前向/后向差分 */
    result.append((n > 1) ? (data[1] - data[0]) / m_dx : 0.0);

    for (int i = 1; i < n - 1; ++i) {
        result.append((data[i + 1] - data[i - 1]) / (2.0 * m_dx));
    }

    if (n > 1) {
        result.append((data[n - 1] - data[n - 2]) / m_dx);
    }
    return result;
}

/** @brief Savitzky-Golay微分(5点二次) @param data 数据 @return 导数 */
QVector<double> NumericalDerivative::diffSavitzkyGolay(
    const QVector<double>& data) const
{
    /* 5点二次SG微分系数 */
    static const double sg5[] = {-2.0, -1.0, 0.0, 1.0, 2.0};
    const double norm = 10.0; /* 归一化因子 */

    QVector<double> result;
    result.reserve(data.size());
    int n = data.size();

    /* 边界用中心差分 */
    for (int i = 0; i < qMin(2, n); ++i) {
        if (i < n - 1) {
            result.append((data[i + 1] - data[i]) / m_dx);
        } else {
            result.append(0.0);
        }
    }

    /* 中间5点SG */
    for (int i = 2; i < n - 2; ++i) {
        double sum = 0.0;
        for (int j = 0; j < 5; ++j) {
            sum += sg5[j] * data[i - 2 + j];
        }
        result.append(sum / (norm * m_dx));
    }

    /* 末尾边界 */
    for (int i = qMax(2, n - 2); i < n; ++i) {
        if (i > 0) {
            result.append((data[i] - data[i - 1]) / m_dx);
        } else {
            result.append(0.0);
        }
    }

    return result;
}
