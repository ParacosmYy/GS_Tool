/**
 * @file algo_3166.cpp
 */
#include "signal3166/algo_3166.h"
QVector<double> algo_3166::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
