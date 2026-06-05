/**
 * @file algo_5026.cpp
 */
#include "signal5026/algo_5026.h"
QVector<double> algo_5026::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
