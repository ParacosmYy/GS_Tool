/**
 * @file algo_4166.cpp
 */
#include "signal4166/algo_4166.h"
QVector<double> algo_4166::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
