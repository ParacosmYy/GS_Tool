/**
 * @file algo_3333.cpp
 */
#include "crypto3333/algo_3333.h"
QVector<double> algo_3333::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
