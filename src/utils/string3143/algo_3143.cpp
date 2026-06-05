/**
 * @file algo_3143.cpp
 */
#include "string3143/algo_3143.h"
QVector<double> algo_3143::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
