/**
 * @file algo_6251.cpp
 */
#include "tree6251/algo_6251.h"
QVector<double> algo_6251::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
