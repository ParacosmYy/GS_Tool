/**
 * @file algo_4278.cpp
 */
#include "neural4278/algo_4278.h"
QVector<double> algo_4278::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
