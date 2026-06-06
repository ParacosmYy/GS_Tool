/**
 * @file algo_6981.cpp
 */
#include "interp6981/algo_6981.h"
QVector<double> algo_6981::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
