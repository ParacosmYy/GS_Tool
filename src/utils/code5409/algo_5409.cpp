/**
 * @file algo_5409.cpp
 */
#include "code5409/algo_5409.h"
QVector<double> algo_5409::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
