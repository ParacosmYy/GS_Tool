/**
 * @file algo_2266.cpp
 * @brief Algorithm module 2266
 */
#include "signal2266/algo_2266.h"
QVector<double> algo_2266::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
