/**
 * @file algo_4589.cpp
 */
#include "code4589/algo_4589.h"
QVector<double> algo_4589::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
