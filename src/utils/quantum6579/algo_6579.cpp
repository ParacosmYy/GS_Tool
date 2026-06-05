/**
 * @file algo_6579.cpp
 */
#include "quantum6579/algo_6579.h"
QVector<double> algo_6579::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
