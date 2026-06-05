/**
 * @file algo_2424.cpp
 * @brief Algorithm module 2424
 */
#include "graph2424/algo_2424.h"
QVector<double> algo_2424::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
