/**
 * @file algo_5212.cpp
 */
#include "compress5212/algo_5212.h"
QVector<double> algo_5212::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
