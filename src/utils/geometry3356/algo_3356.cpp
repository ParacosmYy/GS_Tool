/**
 * @file algo_3356.cpp
 */
#include "geometry3356/algo_3356.h"
QVector<double> algo_3356::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
