/**
 * @file algo_5621.cpp
 */
#include "interp5621/algo_5621.h"
QVector<double> algo_5621::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
