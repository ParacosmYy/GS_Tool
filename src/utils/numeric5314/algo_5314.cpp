/**
 * @file algo_5314.cpp
 */
#include "numeric5314/algo_5314.h"
QVector<double> algo_5314::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
