/**
 * @file algo_6939.cpp
 */
#include "quantum6939/algo_6939.h"
QVector<double> algo_6939::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
