/**
 * @file algo_7186.cpp
 */
#include "signal7186/algo_7186.h"
QVector<double> algo_7186::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
