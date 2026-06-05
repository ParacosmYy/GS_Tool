/**
 * @file algo_6602.cpp
 */
#include "poly6602/algo_6602.h"
QVector<double> algo_6602::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
