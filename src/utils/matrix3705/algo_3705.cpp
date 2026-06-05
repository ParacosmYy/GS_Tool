/**
 * @file algo_3705.cpp
 */
#include "matrix3705/algo_3705.h"
QVector<double> algo_3705::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
