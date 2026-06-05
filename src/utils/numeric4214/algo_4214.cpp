/**
 * @file algo_4214.cpp
 */
#include "numeric4214/algo_4214.h"
QVector<double> algo_4214::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
