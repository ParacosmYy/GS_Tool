/**
 * @file algo_3246.cpp
 */
#include "signal3246/algo_3246.h"
QVector<double> algo_3246::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
