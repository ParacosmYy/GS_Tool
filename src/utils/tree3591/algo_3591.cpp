/**
 * @file algo_3591.cpp
 */
#include "tree3591/algo_3591.h"
QVector<double> algo_3591::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
