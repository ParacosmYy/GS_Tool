/**
 * @file algo_4621.cpp
 */
#include "interp4621/algo_4621.h"
QVector<double> algo_4621::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
