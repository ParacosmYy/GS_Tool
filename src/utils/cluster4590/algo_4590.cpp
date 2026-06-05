/**
 * @file algo_4590.cpp
 */
#include "cluster4590/algo_4590.h"
QVector<double> algo_4590::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
