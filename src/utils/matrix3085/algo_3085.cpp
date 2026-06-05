/**
 * @file algo_3085.cpp
 */
#include "matrix3085/algo_3085.h"
QVector<double> algo_3085::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
