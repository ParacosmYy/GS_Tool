/**
 * @file algo_3960.cpp
 */
#include "sort3960/algo_3960.h"
QVector<double> algo_3960::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
