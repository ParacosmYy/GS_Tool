/**
 * @file algo_1336.cpp
 * @brief Algorithm module 1336
 */
#include "geometry1336/algo_1336.h"
QVector<double> algo_1336::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
