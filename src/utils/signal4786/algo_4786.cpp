/**
 * @file algo_4786.cpp
 */
#include "signal4786/algo_4786.h"
QVector<double> algo_4786::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
