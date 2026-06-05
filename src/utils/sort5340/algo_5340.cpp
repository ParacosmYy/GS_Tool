/**
 * @file algo_5340.cpp
 */
#include "sort5340/algo_5340.h"
QVector<double> algo_5340::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
