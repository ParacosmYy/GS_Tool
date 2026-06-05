/**
 * @file algo_3470.cpp
 */
#include "cluster3470/algo_3470.h"
QVector<double> algo_3470::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
