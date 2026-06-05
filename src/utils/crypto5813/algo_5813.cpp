/**
 * @file algo_5813.cpp
 */
#include "crypto5813/algo_5813.h"
QVector<double> algo_5813::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
