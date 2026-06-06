/**
 * @file algo_7500.cpp
 */
#include "sort7500/algo_7500.h"
QVector<double> algo_7500::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
