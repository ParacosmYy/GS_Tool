/**
 * @file algo_5378.cpp
 */
#include "neural5378/algo_5378.h"
QVector<double> algo_5378::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
