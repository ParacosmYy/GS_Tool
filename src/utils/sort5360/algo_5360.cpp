/**
 * @file algo_5360.cpp
 */
#include "sort5360/algo_5360.h"
QVector<double> algo_5360::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
