/**
 * @file algo_4794.cpp
 */
#include "numeric4794/algo_4794.h"
QVector<double> algo_4794::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
