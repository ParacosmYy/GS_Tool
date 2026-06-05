/**
 * @file algo_2805.cpp
 */
#include "matrix2805/algo_2805.h"
QVector<double> algo_2805::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
