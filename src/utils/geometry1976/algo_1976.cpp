/**
 * @file algo_1976.cpp
 * @brief Algorithm module 1976
 */
#include "geometry1976/algo_1976.h"
QVector<double> algo_1976::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
