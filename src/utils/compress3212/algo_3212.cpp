/**
 * @file algo_3212.cpp
 */
#include "compress3212/algo_3212.h"
QVector<double> algo_3212::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
