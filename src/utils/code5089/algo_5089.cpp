/**
 * @file algo_5089.cpp
 */
#include "code5089/algo_5089.h"
QVector<double> algo_5089::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
