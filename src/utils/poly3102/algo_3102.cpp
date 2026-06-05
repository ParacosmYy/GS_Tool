/**
 * @file algo_3102.cpp
 */
#include "poly3102/algo_3102.h"
QVector<double> algo_3102::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
