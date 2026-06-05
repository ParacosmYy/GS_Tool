/**
 * @file algo_2396.cpp
 * @brief Algorithm module 2396
 */
#include "geometry2396/algo_2396.h"
QVector<double> algo_2396::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
