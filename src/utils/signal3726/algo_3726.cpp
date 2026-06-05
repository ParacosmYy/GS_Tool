/**
 * @file algo_3726.cpp
 */
#include "signal3726/algo_3726.h"
QVector<double> algo_3726::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
