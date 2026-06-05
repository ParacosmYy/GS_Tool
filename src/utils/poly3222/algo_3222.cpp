/**
 * @file algo_3222.cpp
 */
#include "poly3222/algo_3222.h"
QVector<double> algo_3222::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
