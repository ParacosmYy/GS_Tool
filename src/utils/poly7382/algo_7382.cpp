/**
 * @file algo_7382.cpp
 */
#include "poly7382/algo_7382.h"
QVector<double> algo_7382::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
