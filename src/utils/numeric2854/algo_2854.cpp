/**
 * @file algo_2854.cpp
 */
#include "numeric2854/algo_2854.h"
QVector<double> algo_2854::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
