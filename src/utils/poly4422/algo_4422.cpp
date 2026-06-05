/**
 * @file algo_4422.cpp
 */
#include "poly4422/algo_4422.h"
QVector<double> algo_4422::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
