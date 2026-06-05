/**
 * @file algo_4700.cpp
 */
#include "sort4700/algo_4700.h"
QVector<double> algo_4700::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
