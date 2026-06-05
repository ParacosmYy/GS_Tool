/**
 * @file algo_4746.cpp
 */
#include "signal4746/algo_4746.h"
QVector<double> algo_4746::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
