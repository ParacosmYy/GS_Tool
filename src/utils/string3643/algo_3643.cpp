/**
 * @file algo_3643.cpp
 */
#include "string3643/algo_3643.h"
QVector<double> algo_3643::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
