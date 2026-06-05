/**
 * @file algo_6074.cpp
 */
#include "numeric6074/algo_6074.h"
QVector<double> algo_6074::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
