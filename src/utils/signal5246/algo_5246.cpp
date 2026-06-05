/**
 * @file algo_5246.cpp
 */
#include "signal5246/algo_5246.h"
QVector<double> algo_5246::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
