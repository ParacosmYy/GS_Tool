/**
 * @file algo_7741.cpp
 */
#include "interp7741/algo_7741.h"
QVector<double> algo_7741::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
