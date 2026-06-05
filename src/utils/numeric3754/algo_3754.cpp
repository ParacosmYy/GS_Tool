/**
 * @file algo_3754.cpp
 */
#include "numeric3754/algo_3754.h"
QVector<double> algo_3754::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
