/**
 * @file algo_3786.cpp
 */
#include "signal3786/algo_3786.h"
QVector<double> algo_3786::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
