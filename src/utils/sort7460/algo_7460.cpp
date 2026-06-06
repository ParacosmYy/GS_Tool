/**
 * @file algo_7460.cpp
 */
#include "sort7460/algo_7460.h"
QVector<double> algo_7460::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
