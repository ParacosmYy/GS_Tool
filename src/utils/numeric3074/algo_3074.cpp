/**
 * @file algo_3074.cpp
 */
#include "numeric3074/algo_3074.h"
QVector<double> algo_3074::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
