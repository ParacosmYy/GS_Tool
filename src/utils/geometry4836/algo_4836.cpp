/**
 * @file algo_4836.cpp
 */
#include "geometry4836/algo_4836.h"
QVector<double> algo_4836::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
