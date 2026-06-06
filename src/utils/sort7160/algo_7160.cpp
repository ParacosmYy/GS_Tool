/**
 * @file algo_7160.cpp
 */
#include "sort7160/algo_7160.h"
QVector<double> algo_7160::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
