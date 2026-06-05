/**
 * @file algo_3894.cpp
 */
#include "numeric3894/algo_3894.h"
QVector<double> algo_3894::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
