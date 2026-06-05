/**
 * @file algo_3003.cpp
 */
#include "string3003/algo_3003.h"
QVector<double> algo_3003::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
