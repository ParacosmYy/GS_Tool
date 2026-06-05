/**
 * @file algo_4489.cpp
 */
#include "code4489/algo_4489.h"
QVector<double> algo_4489::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
