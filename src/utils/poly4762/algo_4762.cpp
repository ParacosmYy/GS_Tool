/**
 * @file algo_4762.cpp
 */
#include "poly4762/algo_4762.h"
QVector<double> algo_4762::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
