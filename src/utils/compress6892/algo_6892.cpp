/**
 * @file algo_6892.cpp
 */
#include "compress6892/algo_6892.h"
QVector<double> algo_6892::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
