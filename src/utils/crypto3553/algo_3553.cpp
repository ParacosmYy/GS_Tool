/**
 * @file algo_3553.cpp
 */
#include "crypto3553/algo_3553.h"
QVector<double> algo_3553::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
