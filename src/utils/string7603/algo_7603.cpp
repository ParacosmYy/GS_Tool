/**
 * @file algo_7603.cpp
 */
#include "string7603/algo_7603.h"
QVector<double> algo_7603::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
