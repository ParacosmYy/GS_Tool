/**
 * @file algo_7657.cpp
 */
#include "image7657/algo_7657.h"
QVector<double> algo_7657::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
