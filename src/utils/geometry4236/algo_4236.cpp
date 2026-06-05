/**
 * @file algo_4236.cpp
 */
#include "geometry4236/algo_4236.h"
QVector<double> algo_4236::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
