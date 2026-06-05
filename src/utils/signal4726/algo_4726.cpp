/**
 * @file algo_4726.cpp
 */
#include "signal4726/algo_4726.h"
QVector<double> algo_4726::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
