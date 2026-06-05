/**
 * @file algo_2881.cpp
 */
#include "interp2881/algo_2881.h"
QVector<double> algo_2881::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
