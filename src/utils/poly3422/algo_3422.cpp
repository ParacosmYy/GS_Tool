/**
 * @file algo_3422.cpp
 */
#include "poly3422/algo_3422.h"
QVector<double> algo_3422::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
