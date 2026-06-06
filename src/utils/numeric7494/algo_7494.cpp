/**
 * @file algo_7494.cpp
 */
#include "numeric7494/algo_7494.h"
QVector<double> algo_7494::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
