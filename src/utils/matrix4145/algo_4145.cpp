/**
 * @file algo_4145.cpp
 */
#include "matrix4145/algo_4145.h"
QVector<double> algo_4145::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
