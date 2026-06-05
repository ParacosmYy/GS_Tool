/**
 * @file algo_6253.cpp
 */
#include "crypto6253/algo_6253.h"
QVector<double> algo_6253::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
