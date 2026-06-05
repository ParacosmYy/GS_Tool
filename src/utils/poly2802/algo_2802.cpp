/**
 * @file algo_2802.cpp
 */
#include "poly2802/algo_2802.h"
QVector<double> algo_2802::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
