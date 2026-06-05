/**
 * @file algo_1784.cpp
 * @brief Algorithm module 1784
 */
#include "graph1784/algo_1784.h"
QVector<double> algo_1784::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
