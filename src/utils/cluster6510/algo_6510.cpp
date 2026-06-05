/**
 * @file algo_6510.cpp
 */
#include "cluster6510/algo_6510.h"
QVector<double> algo_6510::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
