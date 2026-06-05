/**
 * @file algo_2900.cpp
 */
#include "sort2900/algo_2900.h"
QVector<double> algo_2900::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
