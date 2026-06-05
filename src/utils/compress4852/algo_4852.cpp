/**
 * @file algo_4852.cpp
 */
#include "compress4852/algo_4852.h"
QVector<double> algo_4852::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
