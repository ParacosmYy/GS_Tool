/**
 * @file algo_1984.cpp
 * @brief Algorithm module 1984
 */
#include "graph1984/algo_1984.h"
QVector<double> algo_1984::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
