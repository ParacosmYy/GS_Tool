/**
 * @file algo_5720.cpp
 */
#include "sort5720/algo_5720.h"
QVector<double> algo_5720::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
