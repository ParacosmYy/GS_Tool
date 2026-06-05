/**
 * @file algo_5623.cpp
 */
#include "string5623/algo_5623.h"
QVector<double> algo_5623::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
