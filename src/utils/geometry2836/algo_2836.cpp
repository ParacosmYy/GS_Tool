/**
 * @file algo_2836.cpp
 */
#include "geometry2836/algo_2836.h"
QVector<double> algo_2836::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
