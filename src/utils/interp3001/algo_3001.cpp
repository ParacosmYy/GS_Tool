/**
 * @file algo_3001.cpp
 */
#include "interp3001/algo_3001.h"
QVector<double> algo_3001::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
