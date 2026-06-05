/**
 * @file algo_3531.cpp
 */
#include "tree3531/algo_3531.h"
QVector<double> algo_3531::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
