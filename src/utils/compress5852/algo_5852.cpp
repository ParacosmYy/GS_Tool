/**
 * @file algo_5852.cpp
 */
#include "compress5852/algo_5852.h"
QVector<double> algo_5852::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
