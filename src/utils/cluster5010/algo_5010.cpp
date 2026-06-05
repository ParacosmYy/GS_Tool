/**
 * @file algo_5010.cpp
 */
#include "cluster5010/algo_5010.h"
QVector<double> algo_5010::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
