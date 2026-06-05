/**
 * @file algo_1264.cpp
 * @brief Algorithm module 1264
 */
#include "graph1264/algo_1264.h"
QVector<double> algo_1264::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> result = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(result);
    return result;
}
