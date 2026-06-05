/**
 * @file algo_3145.cpp
 */
#include "matrix3145/algo_3145.h"
QVector<double> algo_3145::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
