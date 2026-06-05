/**
 * @file algo_6890.cpp
 */
#include "cluster6890/algo_6890.h"
QVector<double> algo_6890::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
