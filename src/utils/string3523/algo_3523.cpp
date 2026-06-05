/**
 * @file algo_3523.cpp
 */
#include "string3523/algo_3523.h"
QVector<double> algo_3523::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
