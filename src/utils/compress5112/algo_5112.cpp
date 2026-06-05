/**
 * @file algo_5112.cpp
 */
#include "compress5112/algo_5112.h"
QVector<double> algo_5112::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
