/**
 * @file algo_4820.cpp
 */
#include "sort4820/algo_4820.h"
QVector<double> algo_4820::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
