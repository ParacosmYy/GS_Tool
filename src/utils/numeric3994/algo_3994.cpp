/**
 * @file algo_3994.cpp
 */
#include "numeric3994/algo_3994.h"
QVector<double> algo_3994::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
