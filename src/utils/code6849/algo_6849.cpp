/**
 * @file algo_6849.cpp
 */
#include "code6849/algo_6849.h"
QVector<double> algo_6849::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
