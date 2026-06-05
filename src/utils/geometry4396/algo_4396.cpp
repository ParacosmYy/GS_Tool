/**
 * @file algo_4396.cpp
 */
#include "geometry4396/algo_4396.h"
QVector<double> algo_4396::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
