/**
 * @file algo_4592.cpp
 */
#include "compress4592/algo_4592.h"
QVector<double> algo_4592::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
