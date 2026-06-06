/**
 * @file algo_7103.cpp
 */
#include "string7103/algo_7103.h"
QVector<double> algo_7103::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
