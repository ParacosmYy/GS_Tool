/**
 * @file algo_4502.cpp
 */
#include "poly4502/algo_4502.h"
QVector<double> algo_4502::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
