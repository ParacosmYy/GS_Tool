/**
 * @file algo_3700.cpp
 */
#include "sort3700/algo_3700.h"
QVector<double> algo_3700::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
