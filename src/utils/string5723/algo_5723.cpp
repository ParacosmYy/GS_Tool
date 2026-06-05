/**
 * @file algo_5723.cpp
 */
#include "string5723/algo_5723.h"
QVector<double> algo_5723::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
