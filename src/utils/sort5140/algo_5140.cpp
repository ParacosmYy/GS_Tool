/**
 * @file algo_5140.cpp
 */
#include "sort5140/algo_5140.h"
QVector<double> algo_5140::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
