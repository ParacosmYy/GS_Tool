/**
 * @file algo_7170.cpp
 */
#include "cluster7170/algo_7170.h"
QVector<double> algo_7170::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
