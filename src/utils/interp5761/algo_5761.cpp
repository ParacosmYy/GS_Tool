/**
 * @file algo_5761.cpp
 */
#include "interp5761/algo_5761.h"
QVector<double> algo_5761::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
