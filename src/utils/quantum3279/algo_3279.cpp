/**
 * @file algo_3279.cpp
 */
#include "quantum3279/algo_3279.h"
QVector<double> algo_3279::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
