/**
 * @file algo_4050.cpp
 */
#include "cluster4050/algo_4050.h"
QVector<double> algo_4050::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
