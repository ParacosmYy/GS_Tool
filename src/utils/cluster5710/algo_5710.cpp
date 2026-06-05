/**
 * @file algo_5710.cpp
 */
#include "cluster5710/algo_5710.h"
QVector<double> algo_5710::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
