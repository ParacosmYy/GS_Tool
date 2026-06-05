/**
 * @file algo_4754.cpp
 */
#include "numeric4754/algo_4754.h"
QVector<double> algo_4754::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
