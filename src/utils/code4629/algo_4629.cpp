/**
 * @file algo_4629.cpp
 */
#include "code4629/algo_4629.h"
QVector<double> algo_4629::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
