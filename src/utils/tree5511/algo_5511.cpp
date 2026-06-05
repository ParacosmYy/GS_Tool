/**
 * @file algo_5511.cpp
 */
#include "tree5511/algo_5511.h"
QVector<double> algo_5511::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
