/**
 * @file algo_4085.cpp
 */
#include "matrix4085/algo_4085.h"
QVector<double> algo_4085::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
