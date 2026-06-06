/**
 * @file algo_7223.cpp
 */
#include "string7223/algo_7223.h"
QVector<double> algo_7223::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
