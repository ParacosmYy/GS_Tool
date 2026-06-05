/**
 * @file algo_2912.cpp
 */
#include "compress2912/algo_2912.h"
QVector<double> algo_2912::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
