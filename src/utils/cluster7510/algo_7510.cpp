/**
 * @file algo_7510.cpp
 */
#include "cluster7510/algo_7510.h"
QVector<double> algo_7510::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
