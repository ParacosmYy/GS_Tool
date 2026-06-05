/**
 * @file algo_3225.cpp
 */
#include "matrix3225/algo_3225.h"
QVector<double> algo_3225::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
