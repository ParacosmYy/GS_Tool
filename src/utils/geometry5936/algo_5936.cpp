/**
 * @file algo_5936.cpp
 */
#include "geometry5936/algo_5936.h"
QVector<double> algo_5936::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
