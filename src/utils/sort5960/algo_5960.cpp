/**
 * @file algo_5960.cpp
 */
#include "sort5960/algo_5960.h"
QVector<double> algo_5960::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
