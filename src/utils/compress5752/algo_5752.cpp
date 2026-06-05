/**
 * @file algo_5752.cpp
 */
#include "compress5752/algo_5752.h"
QVector<double> algo_5752::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
