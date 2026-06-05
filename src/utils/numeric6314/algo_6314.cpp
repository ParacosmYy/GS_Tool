/**
 * @file algo_6314.cpp
 */
#include "numeric6314/algo_6314.h"
QVector<double> algo_6314::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
