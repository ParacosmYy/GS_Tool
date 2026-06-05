/**
 * @file algo_3772.cpp
 */
#include "compress3772/algo_3772.h"
QVector<double> algo_3772::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
