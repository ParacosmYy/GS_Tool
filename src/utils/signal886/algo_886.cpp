/**
 * @file algo_886.cpp
 * @brief Algorithm module 886
 */
#include "signal886/algo_886.h"
QVector<double> algo_886::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
