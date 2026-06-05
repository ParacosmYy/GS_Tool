/**
 * @file algo_3300.cpp
 */
#include "sort3300/algo_3300.h"
QVector<double> algo_3300::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
