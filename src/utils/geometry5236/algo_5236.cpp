/**
 * @file algo_5236.cpp
 */
#include "geometry5236/algo_5236.h"
QVector<double> algo_5236::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
