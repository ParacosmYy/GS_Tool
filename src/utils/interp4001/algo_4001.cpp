/**
 * @file algo_4001.cpp
 */
#include "interp4001/algo_4001.h"
QVector<double> algo_4001::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
