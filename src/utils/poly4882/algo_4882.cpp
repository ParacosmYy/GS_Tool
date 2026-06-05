/**
 * @file algo_4882.cpp
 */
#include "poly4882/algo_4882.h"
QVector<double> algo_4882::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
