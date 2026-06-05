/**
 * @file algo_3193.cpp
 */
#include "crypto3193/algo_3193.h"
QVector<double> algo_3193::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
