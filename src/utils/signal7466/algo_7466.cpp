/**
 * @file algo_7466.cpp
 */
#include "signal7466/algo_7466.h"
QVector<double> algo_7466::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
