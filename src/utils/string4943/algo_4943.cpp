/**
 * @file algo_4943.cpp
 */
#include "string4943/algo_4943.h"
QVector<double> algo_4943::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
