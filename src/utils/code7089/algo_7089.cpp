/**
 * @file algo_7089.cpp
 */
#include "code7089/algo_7089.h"
QVector<double> algo_7089::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
