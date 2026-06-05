/**
 * @file algo_6339.cpp
 */
#include "quantum6339/algo_6339.h"
QVector<double> algo_6339::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
