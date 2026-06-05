/**
 * @file algo_3039.cpp
 */
#include "quantum3039/algo_3039.h"
QVector<double> algo_3039::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
