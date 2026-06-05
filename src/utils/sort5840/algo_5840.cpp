/**
 * @file algo_5840.cpp
 */
#include "sort5840/algo_5840.h"
QVector<double> algo_5840::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
