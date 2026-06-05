/**
 * @file algo_4669.cpp
 */
#include "code4669/algo_4669.h"
QVector<double> algo_4669::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
