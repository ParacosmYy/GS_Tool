/**
 * @file algo_4636.cpp
 */
#include "geometry4636/algo_4636.h"
QVector<double> algo_4636::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
