/**
 * @file algo_2478.cpp
 * @brief Algorithm module 2478
 */
#include "neural2478/algo_2478.h"
QVector<double> algo_2478::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
