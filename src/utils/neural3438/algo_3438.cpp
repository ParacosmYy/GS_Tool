/**
 * @file algo_3438.cpp
 */
#include "neural3438/algo_3438.h"
QVector<double> algo_3438::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
