/**
 * @file algo_3800.cpp
 */
#include "sort3800/algo_3800.h"
QVector<double> algo_3800::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
