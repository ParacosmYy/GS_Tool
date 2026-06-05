/**
 * @file algo_4116.cpp
 */
#include "geometry4116/algo_4116.h"
QVector<double> algo_4116::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
