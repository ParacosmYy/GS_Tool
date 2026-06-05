/**
 * @file algo_4152.cpp
 */
#include "compress4152/algo_4152.h"
QVector<double> algo_4152::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
