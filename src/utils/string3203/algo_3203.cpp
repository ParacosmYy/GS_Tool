/**
 * @file algo_3203.cpp
 */
#include "string3203/algo_3203.h"
QVector<double> algo_3203::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
