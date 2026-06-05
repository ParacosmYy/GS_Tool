/**
 * @file algo_2969.cpp
 */
#include "code2969/algo_2969.h"
QVector<double> algo_2969::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
