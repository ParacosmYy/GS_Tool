/**
 * @file algo_6322.cpp
 */
#include "poly6322/algo_6322.h"
QVector<double> algo_6322::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
