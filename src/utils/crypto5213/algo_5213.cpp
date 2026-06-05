/**
 * @file algo_5213.cpp
 */
#include "crypto5213/algo_5213.h"
QVector<double> algo_5213::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
