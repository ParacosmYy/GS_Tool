/**
 * @file algo_4445.cpp
 */
#include "matrix4445/algo_4445.h"
QVector<double> algo_4445::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
