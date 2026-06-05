/**
 * @file algo_3696.cpp
 */
#include "geometry3696/algo_3696.h"
QVector<double> algo_3696::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
