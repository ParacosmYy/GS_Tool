/**
 * @file algo_6485.cpp
 */
#include "matrix6485/algo_6485.h"
QVector<double> algo_6485::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
