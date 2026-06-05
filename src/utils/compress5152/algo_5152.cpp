/**
 * @file algo_5152.cpp
 */
#include "compress5152/algo_5152.h"
QVector<double> algo_5152::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
