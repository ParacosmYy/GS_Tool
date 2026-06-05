/**
 * @file algo_3386.cpp
 */
#include "signal3386/algo_3386.h"
QVector<double> algo_3386::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
