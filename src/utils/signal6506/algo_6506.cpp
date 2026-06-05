/**
 * @file algo_6506.cpp
 */
#include "signal6506/algo_6506.h"
QVector<double> algo_6506::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
