/**
 * @file algo_5234.cpp
 */
#include "numeric5234/algo_5234.h"
QVector<double> algo_5234::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
