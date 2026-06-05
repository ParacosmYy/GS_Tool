/**
 * @file algo_6289.cpp
 */
#include "code6289/algo_6289.h"
QVector<double> algo_6289::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
