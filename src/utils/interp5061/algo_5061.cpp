/**
 * @file algo_5061.cpp
 */
#include "interp5061/algo_5061.h"
QVector<double> algo_5061::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
