/**
 * @file algo_3152.cpp
 */
#include "compress3152/algo_3152.h"
QVector<double> algo_3152::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
