/**
 * @file algo_3341.cpp
 */
#include "interp3341/algo_3341.h"
QVector<double> algo_3341::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
