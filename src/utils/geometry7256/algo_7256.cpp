/**
 * @file algo_7256.cpp
 */
#include "geometry7256/algo_7256.h"
QVector<double> algo_7256::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
