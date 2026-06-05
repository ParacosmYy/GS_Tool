/**
 * @file algo_3552.cpp
 */
#include "compress3552/algo_3552.h"
QVector<double> algo_3552::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
