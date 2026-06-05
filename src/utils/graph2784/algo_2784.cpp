/**
 * @file algo_2784.cpp
 * @brief Algorithm module 2784
 */
#include "graph2784/algo_2784.h"
QVector<double> algo_2784::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
