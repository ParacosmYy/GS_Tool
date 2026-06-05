/**
 * @file algo_2865.cpp
 */
#include "matrix2865/algo_2865.h"
QVector<double> algo_2865::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
