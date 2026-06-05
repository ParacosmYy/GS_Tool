/**
 * @file algo_4686.cpp
 */
#include "signal4686/algo_4686.h"
QVector<double> algo_4686::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
