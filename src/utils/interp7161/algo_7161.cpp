/**
 * @file algo_7161.cpp
 */
#include "interp7161/algo_7161.h"
QVector<double> algo_7161::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
