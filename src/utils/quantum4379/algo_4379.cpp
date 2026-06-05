/**
 * @file algo_4379.cpp
 */
#include "quantum4379/algo_4379.h"
QVector<double> algo_4379::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
