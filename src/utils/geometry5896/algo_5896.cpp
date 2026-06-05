/**
 * @file algo_5896.cpp
 */
#include "geometry5896/algo_5896.h"
QVector<double> algo_5896::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
