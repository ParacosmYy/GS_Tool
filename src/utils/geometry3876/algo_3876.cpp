/**
 * @file algo_3876.cpp
 */
#include "geometry3876/algo_3876.h"
QVector<double> algo_3876::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
