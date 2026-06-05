/**
 * @file algo_6803.cpp
 */
#include "string6803/algo_6803.h"
QVector<double> algo_6803::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
