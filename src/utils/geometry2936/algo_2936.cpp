/**
 * @file algo_2936.cpp
 */
#include "geometry2936/algo_2936.h"
QVector<double> algo_2936::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
