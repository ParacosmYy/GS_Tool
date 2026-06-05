/**
 * @file algo_3006.cpp
 */
#include "signal3006/algo_3006.h"
QVector<double> algo_3006::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
