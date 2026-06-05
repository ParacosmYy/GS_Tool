/**
 * @file algo_5386.cpp
 */
#include "signal5386/algo_5386.h"
QVector<double> algo_5386::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
