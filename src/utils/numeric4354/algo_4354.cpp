/**
 * @file algo_4354.cpp
 */
#include "numeric4354/algo_4354.h"
QVector<double> algo_4354::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
