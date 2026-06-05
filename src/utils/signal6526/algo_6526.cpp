/**
 * @file algo_6526.cpp
 */
#include "signal6526/algo_6526.h"
QVector<double> algo_6526::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
