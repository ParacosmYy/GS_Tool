/**
 * @file algo_5818.cpp
 */
#include "neural5818/algo_5818.h"
QVector<double> algo_5818::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
