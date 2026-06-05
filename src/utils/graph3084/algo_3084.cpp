/**
 * @file algo_3084.cpp
 */
#include "graph3084/algo_3084.h"
QVector<double> algo_3084::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
