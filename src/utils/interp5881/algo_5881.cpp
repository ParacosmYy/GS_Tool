/**
 * @file algo_5881.cpp
 */
#include "interp5881/algo_5881.h"
QVector<double> algo_5881::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
