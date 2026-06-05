/**
 * @file algo_3024.cpp
 */
#include "graph3024/algo_3024.h"
QVector<double> algo_3024::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
