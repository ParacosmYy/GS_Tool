/**
 * @file algo_6705.cpp
 */
#include "matrix6705/algo_6705.h"
QVector<double> algo_6705::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
