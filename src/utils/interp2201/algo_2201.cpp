/**
 * @file algo_2201.cpp
 * @brief Algorithm module 2201
 */
#include "interp2201/algo_2201.h"
QVector<double> algo_2201::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
