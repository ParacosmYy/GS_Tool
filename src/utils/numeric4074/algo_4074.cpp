/**
 * @file algo_4074.cpp
 */
#include "numeric4074/algo_4074.h"
QVector<double> algo_4074::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
