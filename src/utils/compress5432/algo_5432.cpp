/**
 * @file algo_5432.cpp
 */
#include "compress5432/algo_5432.h"
QVector<double> algo_5432::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
