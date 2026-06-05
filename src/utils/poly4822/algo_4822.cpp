/**
 * @file algo_4822.cpp
 */
#include "poly4822/algo_4822.h"
QVector<double> algo_4822::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
