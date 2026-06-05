/**
 * @file algo_1990.cpp
 * @brief Algorithm module 1990
 */
#include "cluster1990/algo_1990.h"
QVector<double> algo_1990::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
