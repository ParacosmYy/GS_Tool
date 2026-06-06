/**
 * @file algo_7120.cpp
 */
#include "sort7120/algo_7120.h"
QVector<double> algo_7120::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
