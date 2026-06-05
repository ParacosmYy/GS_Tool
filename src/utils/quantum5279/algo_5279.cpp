/**
 * @file algo_5279.cpp
 */
#include "quantum5279/algo_5279.h"
QVector<double> algo_5279::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
