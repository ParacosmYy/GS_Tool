/**
 * @file algo_4242.cpp
 */
#include "poly4242/algo_4242.h"
QVector<double> algo_4242::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
