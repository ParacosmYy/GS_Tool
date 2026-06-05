/**
 * @file algo_4704.cpp
 */
#include "graph4704/algo_4704.h"
QVector<double> algo_4704::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
