/**
 * @file algo_6946.cpp
 */
#include "signal6946/algo_6946.h"
QVector<double> algo_6946::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
