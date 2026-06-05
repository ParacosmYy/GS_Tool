/**
 * @file algo_5743.cpp
 */
#include "string5743/algo_5743.h"
QVector<double> algo_5743::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
