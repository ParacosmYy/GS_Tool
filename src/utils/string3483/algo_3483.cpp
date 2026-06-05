/**
 * @file algo_3483.cpp
 */
#include "string3483/algo_3483.h"
QVector<double> algo_3483::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
