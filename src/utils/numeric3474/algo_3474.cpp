/**
 * @file algo_3474.cpp
 */
#include "numeric3474/algo_3474.h"
QVector<double> algo_3474::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
