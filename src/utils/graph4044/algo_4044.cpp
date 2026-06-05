/**
 * @file algo_4044.cpp
 */
#include "graph4044/algo_4044.h"
QVector<double> algo_4044::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
