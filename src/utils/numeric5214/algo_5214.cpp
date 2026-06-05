/**
 * @file algo_5214.cpp
 */
#include "numeric5214/algo_5214.h"
QVector<double> algo_5214::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
