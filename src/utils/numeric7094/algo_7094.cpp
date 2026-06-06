/**
 * @file algo_7094.cpp
 */
#include "numeric7094/algo_7094.h"
QVector<double> algo_7094::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
