/**
 * @file algo_3396.cpp
 */
#include "geometry3396/algo_3396.h"
QVector<double> algo_3396::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
