/**
 * @file algo_3310.cpp
 */
#include "cluster3310/algo_3310.h"
QVector<double> algo_3310::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
