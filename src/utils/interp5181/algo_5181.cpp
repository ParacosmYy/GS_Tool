/**
 * @file algo_5181.cpp
 */
#include "interp5181/algo_5181.h"
QVector<double> algo_5181::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
