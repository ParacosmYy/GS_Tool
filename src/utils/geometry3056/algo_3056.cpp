/**
 * @file algo_3056.cpp
 */
#include "geometry3056/algo_3056.h"
QVector<double> algo_3056::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
