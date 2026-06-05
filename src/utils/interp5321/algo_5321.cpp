/**
 * @file algo_5321.cpp
 */
#include "interp5321/algo_5321.h"
QVector<double> algo_5321::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
