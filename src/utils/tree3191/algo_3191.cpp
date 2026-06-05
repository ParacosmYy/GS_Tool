/**
 * @file algo_3191.cpp
 */
#include "tree3191/algo_3191.h"
QVector<double> algo_3191::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
