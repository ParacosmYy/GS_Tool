/**
 * @file algo_4705.cpp
 */
#include "matrix4705/algo_4705.h"
QVector<double> algo_4705::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
