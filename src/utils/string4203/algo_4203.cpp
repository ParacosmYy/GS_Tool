/**
 * @file algo_4203.cpp
 */
#include "string4203/algo_4203.h"
QVector<double> algo_4203::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
