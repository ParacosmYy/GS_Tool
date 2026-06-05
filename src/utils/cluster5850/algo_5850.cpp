/**
 * @file algo_5850.cpp
 */
#include "cluster5850/algo_5850.h"
QVector<double> algo_5850::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
