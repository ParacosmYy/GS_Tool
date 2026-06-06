/**
 * @file algo_7041.cpp
 */
#include "interp7041/algo_7041.h"
QVector<double> algo_7041::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
