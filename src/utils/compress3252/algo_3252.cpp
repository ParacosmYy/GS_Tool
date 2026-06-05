/**
 * @file algo_3252.cpp
 */
#include "compress3252/algo_3252.h"
QVector<double> algo_3252::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
