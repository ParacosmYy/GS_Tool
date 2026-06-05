/**
 * @file algo_2304.cpp
 * @brief Algorithm module 2304
 */
#include "graph2304/algo_2304.h"
QVector<double> algo_2304::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
