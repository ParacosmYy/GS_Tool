/**
 * @file algo_3091.cpp
 */
#include "tree3091/algo_3091.h"
QVector<double> algo_3091::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
