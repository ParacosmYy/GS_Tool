/**
 * @file algo_7203.cpp
 */
#include "string7203/algo_7203.h"
QVector<double> algo_7203::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
