/**
 * @file algo_5177.cpp
 */
#include "image5177/algo_5177.h"
QVector<double> algo_5177::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
