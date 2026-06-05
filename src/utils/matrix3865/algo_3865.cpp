/**
 * @file algo_3865.cpp
 */
#include "matrix3865/algo_3865.h"
QVector<double> algo_3865::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
