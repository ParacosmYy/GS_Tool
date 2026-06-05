/**
 * @file algo_6286.cpp
 */
#include "signal6286/algo_6286.h"
QVector<double> algo_6286::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
