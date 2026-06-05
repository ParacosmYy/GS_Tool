/**
 * @file algo_5732.cpp
 */
#include "compress5732/algo_5732.h"
QVector<double> algo_5732::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
