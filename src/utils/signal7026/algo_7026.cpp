/**
 * @file algo_7026.cpp
 */
#include "signal7026/algo_7026.h"
QVector<double> algo_7026::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
