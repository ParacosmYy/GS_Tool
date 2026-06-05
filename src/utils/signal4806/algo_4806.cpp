/**
 * @file algo_4806.cpp
 */
#include "signal4806/algo_4806.h"
QVector<double> algo_4806::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
