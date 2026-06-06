/**
 * @file algo_7520.cpp
 */
#include "sort7520/algo_7520.h"
QVector<double> algo_7520::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
