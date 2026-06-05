/**
 * @file algo_6681.cpp
 */
#include "interp6681/algo_6681.h"
QVector<double> algo_6681::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
