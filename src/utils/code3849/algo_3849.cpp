/**
 * @file algo_3849.cpp
 */
#include "code3849/algo_3849.h"
QVector<double> algo_3849::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
