/**
 * @file algo_5874.cpp
 */
#include "numeric5874/algo_5874.h"
QVector<double> algo_5874::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
