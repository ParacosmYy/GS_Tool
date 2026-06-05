/**
 * @file algo_5500.cpp
 */
#include "sort5500/algo_5500.h"
QVector<double> algo_5500::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
