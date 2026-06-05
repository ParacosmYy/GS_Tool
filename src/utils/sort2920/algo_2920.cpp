/**
 * @file algo_2920.cpp
 */
#include "sort2920/algo_2920.h"
QVector<double> algo_2920::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
