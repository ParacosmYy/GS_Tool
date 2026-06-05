/**
 * @file algo_2894.cpp
 */
#include "numeric2894/algo_2894.h"
QVector<double> algo_2894::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
