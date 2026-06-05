/**
 * @file algo_6802.cpp
 */
#include "poly6802/algo_6802.h"
QVector<double> algo_6802::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
