/**
 * @file algo_3223.cpp
 */
#include "string3223/algo_3223.h"
QVector<double> algo_3223::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
