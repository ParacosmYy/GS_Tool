/**
 * @file algo_7082.cpp
 */
#include "poly7082/algo_7082.h"
QVector<double> algo_7082::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
