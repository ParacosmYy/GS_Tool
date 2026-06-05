/**
 * @file algo_4939.cpp
 */
#include "quantum4939/algo_4939.h"
QVector<double> algo_4939::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
