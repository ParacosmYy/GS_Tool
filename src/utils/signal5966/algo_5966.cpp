/**
 * @file algo_5966.cpp
 */
#include "signal5966/algo_5966.h"
QVector<double> algo_5966::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
