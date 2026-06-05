/**
 * @file algo_3804.cpp
 */
#include "graph3804/algo_3804.h"
QVector<double> algo_3804::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
