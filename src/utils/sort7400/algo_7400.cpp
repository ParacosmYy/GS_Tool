/**
 * @file algo_7400.cpp
 */
#include "sort7400/algo_7400.h"
QVector<double> algo_7400::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
