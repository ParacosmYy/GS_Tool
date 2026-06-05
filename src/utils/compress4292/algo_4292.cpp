/**
 * @file algo_4292.cpp
 */
#include "compress4292/algo_4292.h"
QVector<double> algo_4292::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
