/**
 * @file algo_5242.cpp
 */
#include "poly5242/algo_5242.h"
QVector<double> algo_5242::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
