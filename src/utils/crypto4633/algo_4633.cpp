/**
 * @file algo_4633.cpp
 */
#include "crypto4633/algo_4633.h"
QVector<double> algo_4633::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
