/**
 * @file algo_6161.cpp
 */
#include "interp6161/algo_6161.h"
QVector<double> algo_6161::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
