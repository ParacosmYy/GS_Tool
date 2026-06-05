/**
 * @file algo_6442.cpp
 */
#include "poly6442/algo_6442.h"
QVector<double> algo_6442::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
