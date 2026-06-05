/**
 * @file algo_6126.cpp
 */
#include "signal6126/algo_6126.h"
QVector<double> algo_6126::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
