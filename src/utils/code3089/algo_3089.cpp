/**
 * @file algo_3089.cpp
 */
#include "code3089/algo_3089.h"
QVector<double> algo_3089::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
