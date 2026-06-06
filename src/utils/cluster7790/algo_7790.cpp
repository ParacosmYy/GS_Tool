/**
 * @file algo_7790.cpp
 */
#include "cluster7790/algo_7790.h"
QVector<double> algo_7790::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
