/**
 * @file algo_5174.cpp
 */
#include "numeric5174/algo_5174.h"
QVector<double> algo_5174::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
