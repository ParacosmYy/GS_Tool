/**
 * @file algo_7586.cpp
 */
#include "signal7586/algo_7586.h"
QVector<double> algo_7586::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
