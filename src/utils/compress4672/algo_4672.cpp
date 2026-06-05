/**
 * @file algo_4672.cpp
 */
#include "compress4672/algo_4672.h"
QVector<double> algo_4672::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
