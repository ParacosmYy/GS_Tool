/**
 * @file algo_4202.cpp
 */
#include "poly4202/algo_4202.h"
QVector<double> algo_4202::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
