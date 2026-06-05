/**
 * @file algo_6590.cpp
 */
#include "cluster6590/algo_6590.h"
QVector<double> algo_6590::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
