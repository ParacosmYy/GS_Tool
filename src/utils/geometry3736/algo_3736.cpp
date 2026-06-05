/**
 * @file algo_3736.cpp
 */
#include "geometry3736/algo_3736.h"
QVector<double> algo_3736::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
