/**
 * @file algo_3760.cpp
 */
#include "sort3760/algo_3760.h"
QVector<double> algo_3760::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
