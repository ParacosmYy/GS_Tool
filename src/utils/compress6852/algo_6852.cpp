/**
 * @file algo_6852.cpp
 */
#include "compress6852/algo_6852.h"
QVector<double> algo_6852::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
