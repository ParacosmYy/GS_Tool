/**
 * @file algo_4804.cpp
 */
#include "graph4804/algo_4804.h"
QVector<double> algo_4804::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
