/**
 * @file algo_7214.cpp
 */
#include "numeric7214/algo_7214.h"
QVector<double> algo_7214::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
