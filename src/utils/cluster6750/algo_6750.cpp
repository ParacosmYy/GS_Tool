/**
 * @file algo_6750.cpp
 */
#include "cluster6750/algo_6750.h"
QVector<double> algo_6750::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
