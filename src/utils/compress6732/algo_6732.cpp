/**
 * @file algo_6732.cpp
 */
#include "compress6732/algo_6732.h"
QVector<double> algo_6732::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
