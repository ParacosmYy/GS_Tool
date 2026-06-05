/**
 * @file algo_5726.cpp
 */
#include "signal5726/algo_5726.h"
QVector<double> algo_5726::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
