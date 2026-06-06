/**
 * @file algo_7560.cpp
 */
#include "sort7560/algo_7560.h"
QVector<double> algo_7560::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
