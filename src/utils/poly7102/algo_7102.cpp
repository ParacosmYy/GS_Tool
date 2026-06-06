/**
 * @file algo_7102.cpp
 */
#include "poly7102/algo_7102.h"
QVector<double> algo_7102::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
