/**
 * @file algo_4232.cpp
 */
#include "compress4232/algo_4232.h"
QVector<double> algo_4232::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
