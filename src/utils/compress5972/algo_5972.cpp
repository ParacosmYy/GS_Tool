/**
 * @file algo_5972.cpp
 */
#include "compress5972/algo_5972.h"
QVector<double> algo_5972::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
