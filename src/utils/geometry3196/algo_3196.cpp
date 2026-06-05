/**
 * @file algo_3196.cpp
 */
#include "geometry3196/algo_3196.h"
QVector<double> algo_3196::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
