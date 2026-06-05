/**
 * @file algo_6729.cpp
 */
#include "code6729/algo_6729.h"
QVector<double> algo_6729::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
