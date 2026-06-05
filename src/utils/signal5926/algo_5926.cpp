/**
 * @file algo_5926.cpp
 */
#include "signal5926/algo_5926.h"
QVector<double> algo_5926::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
