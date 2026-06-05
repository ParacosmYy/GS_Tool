/**
 * @file algo_3153.cpp
 */
#include "crypto3153/algo_3153.h"
QVector<double> algo_3153::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
