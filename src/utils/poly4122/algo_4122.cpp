/**
 * @file algo_4122.cpp
 */
#include "poly4122/algo_4122.h"
QVector<double> algo_4122::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
