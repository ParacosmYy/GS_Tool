/**
 * @file algo_6882.cpp
 */
#include "poly6882/algo_6882.h"
QVector<double> algo_6882::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
