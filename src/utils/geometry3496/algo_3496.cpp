/**
 * @file algo_3496.cpp
 */
#include "geometry3496/algo_3496.h"
QVector<double> algo_3496::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
