/**
 * @file algo_4124.cpp
 */
#include "graph4124/algo_4124.h"
QVector<double> algo_4124::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
