/**
 * @file algo_3933.cpp
 */
#include "crypto3933/algo_3933.h"
QVector<double> algo_3933::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
