/**
 * @file algo_6060.cpp
 */
#include "sort6060/algo_6060.h"
QVector<double> algo_6060::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
