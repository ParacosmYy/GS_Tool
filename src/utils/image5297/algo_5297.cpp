/**
 * @file algo_5297.cpp
 */
#include "image5297/algo_5297.h"
QVector<double> algo_5297::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
