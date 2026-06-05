/**
 * @file algo_3556.cpp
 */
#include "geometry3556/algo_3556.h"
QVector<double> algo_3556::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
