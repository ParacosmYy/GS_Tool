/**
 * @file algo_7037.cpp
 */
#include "image7037/algo_7037.h"
QVector<double> algo_7037::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
