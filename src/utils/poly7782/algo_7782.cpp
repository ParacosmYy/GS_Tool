/**
 * @file algo_7782.cpp
 */
#include "poly7782/algo_7782.h"
QVector<double> algo_7782::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
