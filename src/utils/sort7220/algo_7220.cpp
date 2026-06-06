/**
 * @file algo_7220.cpp
 */
#include "sort7220/algo_7220.h"
QVector<double> algo_7220::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
