/**
 * @file algo_7242.cpp
 */
#include "poly7242/algo_7242.h"
QVector<double> algo_7242::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
