/**
 * @file algo_4389.cpp
 */
#include "code4389/algo_4389.h"
QVector<double> algo_4389::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
