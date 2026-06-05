/**
 * @file algo_4095.cpp
 */
#include "optim4095/algo_4095.h"
QVector<double> algo_4095::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
