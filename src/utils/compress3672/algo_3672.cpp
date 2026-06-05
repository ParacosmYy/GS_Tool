/**
 * @file algo_3672.cpp
 */
#include "compress3672/algo_3672.h"
QVector<double> algo_3672::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
