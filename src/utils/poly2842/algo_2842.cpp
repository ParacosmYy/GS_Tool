/**
 * @file algo_2842.cpp
 */
#include "poly2842/algo_2842.h"
QVector<double> algo_2842::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
