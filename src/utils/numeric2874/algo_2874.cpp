/**
 * @file algo_2874.cpp
 */
#include "numeric2874/algo_2874.h"
QVector<double> algo_2874::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
