/**
 * @file algo_6226.cpp
 */
#include "signal6226/algo_6226.h"
QVector<double> algo_6226::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
