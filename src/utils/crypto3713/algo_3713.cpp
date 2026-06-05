/**
 * @file algo_3713.cpp
 */
#include "crypto3713/algo_3713.h"
QVector<double> algo_3713::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
