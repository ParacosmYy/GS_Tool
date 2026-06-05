/**
 * @file algo_5097.cpp
 */
#include "image5097/algo_5097.h"
QVector<double> algo_5097::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
