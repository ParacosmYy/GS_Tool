/**
 * @file algo_7780.cpp
 */
#include "sort7780/algo_7780.h"
QVector<double> algo_7780::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
