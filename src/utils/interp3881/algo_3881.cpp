/**
 * @file algo_3881.cpp
 */
#include "interp3881/algo_3881.h"
QVector<double> algo_3881::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
