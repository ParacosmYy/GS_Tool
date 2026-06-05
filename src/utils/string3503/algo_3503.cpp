/**
 * @file algo_3503.cpp
 */
#include "string3503/algo_3503.h"
QVector<double> algo_3503::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
