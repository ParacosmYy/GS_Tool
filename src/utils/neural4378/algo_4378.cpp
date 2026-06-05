/**
 * @file algo_4378.cpp
 */
#include "neural4378/algo_4378.h"
QVector<double> algo_4378::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
