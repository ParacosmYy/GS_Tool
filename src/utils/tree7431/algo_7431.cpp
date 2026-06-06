/**
 * @file algo_7431.cpp
 */
#include "tree7431/algo_7431.h"
QVector<double> algo_7431::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
