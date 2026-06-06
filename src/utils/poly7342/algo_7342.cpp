/**
 * @file algo_7342.cpp
 */
#include "poly7342/algo_7342.h"
QVector<double> algo_7342::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
