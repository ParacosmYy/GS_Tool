/**
 * @file algo_3390.cpp
 */
#include "cluster3390/algo_3390.h"
QVector<double> algo_3390::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
