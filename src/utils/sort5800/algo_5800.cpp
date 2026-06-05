/**
 * @file algo_5800.cpp
 */
#include "sort5800/algo_5800.h"
QVector<double> algo_5800::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
