/**
 * @file algo_4103.cpp
 */
#include "string4103/algo_4103.h"
QVector<double> algo_4103::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
