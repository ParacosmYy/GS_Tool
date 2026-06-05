/**
 * @file algo_5889.cpp
 */
#include "code5889/algo_5889.h"
QVector<double> algo_5889::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
