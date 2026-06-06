/**
 * @file algo_7200.cpp
 */
#include "sort7200/algo_7200.h"
QVector<double> algo_7200::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
