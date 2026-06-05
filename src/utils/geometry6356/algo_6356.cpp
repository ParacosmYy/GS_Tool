/**
 * @file algo_6356.cpp
 */
#include "geometry6356/algo_6356.h"
QVector<double> algo_6356::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
