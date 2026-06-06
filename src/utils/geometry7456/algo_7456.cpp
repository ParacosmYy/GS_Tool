/**
 * @file algo_7456.cpp
 */
#include "geometry7456/algo_7456.h"
QVector<double> algo_7456::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
