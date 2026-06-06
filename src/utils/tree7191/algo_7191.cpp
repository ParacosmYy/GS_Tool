/**
 * @file algo_7191.cpp
 */
#include "tree7191/algo_7191.h"
QVector<double> algo_7191::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
