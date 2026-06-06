/**
 * @file algo_7771.cpp
 */
#include "tree7771/algo_7771.h"
QVector<double> algo_7771::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
