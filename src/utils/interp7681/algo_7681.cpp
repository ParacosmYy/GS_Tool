/**
 * @file algo_7681.cpp
 */
#include "interp7681/algo_7681.h"
QVector<double> algo_7681::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
