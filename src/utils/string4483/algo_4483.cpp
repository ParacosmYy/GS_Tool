/**
 * @file algo_4483.cpp
 */
#include "string4483/algo_4483.h"
QVector<double> algo_4483::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
