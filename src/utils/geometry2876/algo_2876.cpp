/**
 * @file algo_2876.cpp
 */
#include "geometry2876/algo_2876.h"
QVector<double> algo_2876::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
