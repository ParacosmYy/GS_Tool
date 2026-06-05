/**
 * @file algo_3192.cpp
 */
#include "compress3192/algo_3192.h"
QVector<double> algo_3192::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
