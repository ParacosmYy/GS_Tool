/**
 * @file algo_7561.cpp
 */
#include "interp7561/algo_7561.h"
QVector<double> algo_7561::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
