/**
 * @file algo_7486.cpp
 */
#include "signal7486/algo_7486.h"
QVector<double> algo_7486::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
