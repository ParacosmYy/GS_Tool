/**
 * @file algo_3681.cpp
 */
#include "interp3681/algo_3681.h"
QVector<double> algo_3681::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
