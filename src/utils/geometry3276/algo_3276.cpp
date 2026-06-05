/**
 * @file algo_3276.cpp
 */
#include "geometry3276/algo_3276.h"
QVector<double> algo_3276::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
