/**
 * @file algo_7166.cpp
 */
#include "signal7166/algo_7166.h"
QVector<double> algo_7166::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
