/**
 * @file algo_4909.cpp
 */
#include "code4909/algo_4909.h"
QVector<double> algo_4909::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
