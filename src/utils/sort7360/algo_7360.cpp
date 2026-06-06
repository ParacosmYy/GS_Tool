/**
 * @file algo_7360.cpp
 */
#include "sort7360/algo_7360.h"
QVector<double> algo_7360::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
