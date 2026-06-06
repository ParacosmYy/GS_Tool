/**
 * @file algo_7640.cpp
 */
#include "sort7640/algo_7640.h"
QVector<double> algo_7640::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
