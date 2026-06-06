/**
 * @file algo_7000.cpp
 */
#include "sort7000/algo_7000.h"
QVector<double> algo_7000::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
