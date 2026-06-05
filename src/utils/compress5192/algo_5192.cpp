/**
 * @file algo_5192.cpp
 */
#include "compress5192/algo_5192.h"
QVector<double> algo_5192::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
