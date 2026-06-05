/**
 * @file algo_3616.cpp
 */
#include "geometry3616/algo_3616.h"
QVector<double> algo_3616::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
