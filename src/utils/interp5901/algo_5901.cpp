/**
 * @file algo_5901.cpp
 */
#include "interp5901/algo_5901.h"
QVector<double> algo_5901::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
