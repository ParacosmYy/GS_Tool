/**
 * @file algo_6586.cpp
 */
#include "signal6586/algo_6586.h"
QVector<double> algo_6586::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
