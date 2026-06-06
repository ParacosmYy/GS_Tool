/**
 * @file algo_7221.cpp
 */
#include "interp7221/algo_7221.h"
QVector<double> algo_7221::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
