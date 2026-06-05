/**
 * @file algo_2861.cpp
 */
#include "interp2861/algo_2861.h"
QVector<double> algo_2861::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
