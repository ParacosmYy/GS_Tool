/**
 * @file algo_4336.cpp
 */
#include "geometry4336/algo_4336.h"
QVector<double> algo_4336::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
