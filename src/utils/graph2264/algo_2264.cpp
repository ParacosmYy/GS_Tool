/**
 * @file algo_2264.cpp
 * @brief Algorithm module 2264
 */
#include "graph2264/algo_2264.h"
QVector<double> algo_2264::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
