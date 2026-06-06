/**
 * @file algo_7422.cpp
 */
#include "poly7422/algo_7422.h"
QVector<double> algo_7422::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
