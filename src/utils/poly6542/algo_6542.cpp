/**
 * @file algo_6542.cpp
 */
#include "poly6542/algo_6542.h"
QVector<double> algo_6542::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
