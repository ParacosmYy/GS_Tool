/**
 * @file algo_7631.cpp
 */
#include "tree7631/algo_7631.h"
QVector<double> algo_7631::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
