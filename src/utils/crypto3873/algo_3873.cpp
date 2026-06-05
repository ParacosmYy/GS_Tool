/**
 * @file algo_3873.cpp
 */
#include "crypto3873/algo_3873.h"
QVector<double> algo_3873::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
