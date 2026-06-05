/**
 * @file algo_6061.cpp
 */
#include "interp6061/algo_6061.h"
QVector<double> algo_6061::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
