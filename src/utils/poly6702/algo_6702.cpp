/**
 * @file algo_6702.cpp
 */
#include "poly6702/algo_6702.h"
QVector<double> algo_6702::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
