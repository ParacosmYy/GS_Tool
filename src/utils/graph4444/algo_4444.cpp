/**
 * @file algo_4444.cpp
 */
#include "graph4444/algo_4444.h"
QVector<double> algo_4444::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
