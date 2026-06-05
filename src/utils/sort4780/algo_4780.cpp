/**
 * @file algo_4780.cpp
 */
#include "sort4780/algo_4780.h"
QVector<double> algo_4780::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
