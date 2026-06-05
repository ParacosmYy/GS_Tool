/**
 * @file algo_2941.cpp
 */
#include "interp2941/algo_2941.h"
QVector<double> algo_2941::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
