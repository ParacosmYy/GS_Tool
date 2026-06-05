/**
 * @file algo_6441.cpp
 */
#include "interp6441/algo_6441.h"
QVector<double> algo_6441::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
