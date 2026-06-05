/**
 * @file algo_6086.cpp
 */
#include "signal6086/algo_6086.h"
QVector<double> algo_6086::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
