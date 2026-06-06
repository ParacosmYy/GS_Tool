/**
 * @file algo_7777.cpp
 */
#include "image7777/algo_7777.h"
QVector<double> algo_7777::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
