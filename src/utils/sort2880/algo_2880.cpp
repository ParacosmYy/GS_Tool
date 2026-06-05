/**
 * @file algo_2880.cpp
 */
#include "sort2880/algo_2880.h"
QVector<double> algo_2880::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
