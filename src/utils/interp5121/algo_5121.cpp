/**
 * @file algo_5121.cpp
 */
#include "interp5121/algo_5121.h"
QVector<double> algo_5121::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
