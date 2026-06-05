/**
 * @file algo_4289.cpp
 */
#include "code4289/algo_4289.h"
QVector<double> algo_4289::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
