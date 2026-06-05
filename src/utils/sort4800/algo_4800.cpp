/**
 * @file algo_4800.cpp
 */
#include "sort4800/algo_4800.h"
QVector<double> algo_4800::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
