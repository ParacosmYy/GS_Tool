/**
 * @file algo_4883.cpp
 */
#include "string4883/algo_4883.h"
QVector<double> algo_4883::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
