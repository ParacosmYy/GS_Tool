/**
 * @file algo_3393.cpp
 */
#include "crypto3393/algo_3393.h"
QVector<double> algo_3393::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
