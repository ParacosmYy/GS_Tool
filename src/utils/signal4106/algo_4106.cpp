/**
 * @file algo_4106.cpp
 */
#include "signal4106/algo_4106.h"
QVector<double> algo_4106::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
