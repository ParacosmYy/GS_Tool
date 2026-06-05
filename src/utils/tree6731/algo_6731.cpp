/**
 * @file algo_6731.cpp
 */
#include "tree6731/algo_6731.h"
QVector<double> algo_6731::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
