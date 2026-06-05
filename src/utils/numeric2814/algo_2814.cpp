/**
 * @file algo_2814.cpp
 */
#include "numeric2814/algo_2814.h"
QVector<double> algo_2814::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
