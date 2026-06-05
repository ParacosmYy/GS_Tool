/**
 * @file algo_4269.cpp
 */
#include "code4269/algo_4269.h"
QVector<double> algo_4269::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
