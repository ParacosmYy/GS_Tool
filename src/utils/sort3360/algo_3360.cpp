/**
 * @file algo_3360.cpp
 */
#include "sort3360/algo_3360.h"
QVector<double> algo_3360::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
