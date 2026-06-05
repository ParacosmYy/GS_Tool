/**
 * @file algo_3580.cpp
 */
#include "sort3580/algo_3580.h"
QVector<double> algo_3580::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
