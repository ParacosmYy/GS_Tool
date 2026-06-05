/**
 * @file algo_6382.cpp
 */
#include "poly6382/algo_6382.h"
QVector<double> algo_6382::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
