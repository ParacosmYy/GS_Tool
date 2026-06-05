/**
 * @file algo_3141.cpp
 */
#include "interp3141/algo_3141.h"
QVector<double> algo_3141::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
