/**
 * @file algo_6152.cpp
 */
#include "compress6152/algo_6152.h"
QVector<double> algo_6152::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
