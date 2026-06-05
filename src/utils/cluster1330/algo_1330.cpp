/**
 * @file algo_1330.cpp
 * @brief Algorithm module 1330
 */
#include "cluster1330/algo_1330.h"
QVector<double> algo_1330::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
