/**
 * @file algo_5959.cpp
 */
#include "quantum5959/algo_5959.h"
QVector<double> algo_5959::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
