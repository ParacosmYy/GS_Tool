/**
 * @file algo_5780.cpp
 */
#include "sort5780/algo_5780.h"
QVector<double> algo_5780::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
