/**
 * @file algo_990.cpp
 * @brief Algorithm module 990
 */
#include "cluster990/algo_990.h"
QVector<double> algo_990::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
