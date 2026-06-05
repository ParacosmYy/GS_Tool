/**
 * @file algo_5310.cpp
 */
#include "cluster5310/algo_5310.h"
QVector<double> algo_5310::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
