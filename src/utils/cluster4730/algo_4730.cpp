/**
 * @file algo_4730.cpp
 */
#include "cluster4730/algo_4730.h"
QVector<double> algo_4730::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
