/**
 * @file algo_7217.cpp
 */
#include "image7217/algo_7217.h"
QVector<double> algo_7217::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
