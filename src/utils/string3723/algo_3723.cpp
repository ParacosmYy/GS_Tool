/**
 * @file algo_3723.cpp
 */
#include "string3723/algo_3723.h"
QVector<double> algo_3723::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
