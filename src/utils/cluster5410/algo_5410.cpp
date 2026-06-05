/**
 * @file algo_5410.cpp
 */
#include "cluster5410/algo_5410.h"
QVector<double> algo_5410::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
