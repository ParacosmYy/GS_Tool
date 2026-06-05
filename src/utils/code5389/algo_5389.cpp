/**
 * @file algo_5389.cpp
 */
#include "code5389/algo_5389.h"
QVector<double> algo_5389::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
