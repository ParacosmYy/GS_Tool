/**
 * @file algo_3373.cpp
 */
#include "crypto3373/algo_3373.h"
QVector<double> algo_3373::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
