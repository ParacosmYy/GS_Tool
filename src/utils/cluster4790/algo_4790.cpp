/**
 * @file algo_4790.cpp
 */
#include "cluster4790/algo_4790.h"
QVector<double> algo_4790::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
