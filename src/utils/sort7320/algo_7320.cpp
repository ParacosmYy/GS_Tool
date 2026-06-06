/**
 * @file algo_7320.cpp
 */
#include "sort7320/algo_7320.h"
QVector<double> algo_7320::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
