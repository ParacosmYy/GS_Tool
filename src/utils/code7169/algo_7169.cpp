/**
 * @file algo_7169.cpp
 */
#include "code7169/algo_7169.h"
QVector<double> algo_7169::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
