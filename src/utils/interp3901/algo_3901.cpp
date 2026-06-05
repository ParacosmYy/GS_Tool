/**
 * @file algo_3901.cpp
 */
#include "interp3901/algo_3901.h"
QVector<double> algo_3901::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
