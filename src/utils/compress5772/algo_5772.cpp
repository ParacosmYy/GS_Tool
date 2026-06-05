/**
 * @file algo_5772.cpp
 */
#include "compress5772/algo_5772.h"
QVector<double> algo_5772::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
