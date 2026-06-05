/**
 * @file algo_3379.cpp
 */
#include "quantum3379/algo_3379.h"
QVector<double> algo_3379::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
