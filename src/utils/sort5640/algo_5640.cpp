/**
 * @file algo_5640.cpp
 */
#include "sort5640/algo_5640.h"
QVector<double> algo_5640::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
