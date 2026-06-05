/**
 * @file algo_6772.cpp
 */
#include "compress6772/algo_6772.h"
QVector<double> algo_6772::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
