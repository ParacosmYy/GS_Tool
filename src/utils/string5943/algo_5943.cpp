/**
 * @file algo_5943.cpp
 */
#include "string5943/algo_5943.h"
QVector<double> algo_5943::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
