/**
 * @file algo_4510.cpp
 */
#include "cluster4510/algo_4510.h"
QVector<double> algo_4510::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
