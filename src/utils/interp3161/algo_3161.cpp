/**
 * @file algo_3161.cpp
 */
#include "interp3161/algo_3161.h"
QVector<double> algo_3161::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
