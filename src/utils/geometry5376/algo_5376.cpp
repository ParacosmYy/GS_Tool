/**
 * @file algo_5376.cpp
 */
#include "geometry5376/algo_5376.h"
QVector<double> algo_5376::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
