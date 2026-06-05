/**
 * @file algo_3476.cpp
 */
#include "geometry3476/algo_3476.h"
QVector<double> algo_3476::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
