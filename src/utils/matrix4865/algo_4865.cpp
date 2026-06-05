/**
 * @file algo_4865.cpp
 */
#include "matrix4865/algo_4865.h"
QVector<double> algo_4865::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
