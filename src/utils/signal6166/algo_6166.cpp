/**
 * @file algo_6166.cpp
 */
#include "signal6166/algo_6166.h"
QVector<double> algo_6166::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
