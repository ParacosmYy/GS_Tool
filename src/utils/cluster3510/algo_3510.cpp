/**
 * @file algo_3510.cpp
 */
#include "cluster3510/algo_3510.h"
QVector<double> algo_3510::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
