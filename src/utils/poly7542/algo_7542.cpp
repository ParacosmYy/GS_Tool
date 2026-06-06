/**
 * @file algo_7542.cpp
 */
#include "poly7542/algo_7542.h"
QVector<double> algo_7542::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
