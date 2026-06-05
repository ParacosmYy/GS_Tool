/**
 * @file algo_3776.cpp
 */
#include "geometry3776/algo_3776.h"
QVector<double> algo_3776::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
