/**
 * @file algo_4386.cpp
 */
#include "signal4386/algo_4386.h"
QVector<double> algo_4386::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
