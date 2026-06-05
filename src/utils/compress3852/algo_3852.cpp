/**
 * @file algo_3852.cpp
 */
#include "compress3852/algo_3852.h"
QVector<double> algo_3852::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
