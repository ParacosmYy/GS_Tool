/**
 * @file algo_4936.cpp
 */
#include "geometry4936/algo_4936.h"
QVector<double> algo_4936::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
