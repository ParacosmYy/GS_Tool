/**
 * @file algo_7194.cpp
 */
#include "numeric7194/algo_7194.h"
QVector<double> algo_7194::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
