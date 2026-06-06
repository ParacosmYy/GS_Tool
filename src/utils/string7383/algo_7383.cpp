/**
 * @file algo_7383.cpp
 */
#include "string7383/algo_7383.h"
QVector<double> algo_7383::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
