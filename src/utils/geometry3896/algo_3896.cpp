/**
 * @file algo_3896.cpp
 */
#include "geometry3896/algo_3896.h"
QVector<double> algo_3896::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
