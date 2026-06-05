/**
 * @file algo_3260.cpp
 */
#include "sort3260/algo_3260.h"
QVector<double> algo_3260::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
