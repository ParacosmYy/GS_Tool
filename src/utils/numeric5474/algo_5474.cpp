/**
 * @file algo_5474.cpp
 */
#include "numeric5474/algo_5474.h"
QVector<double> algo_5474::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
