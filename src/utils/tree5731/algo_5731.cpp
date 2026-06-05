/**
 * @file algo_5731.cpp
 */
#include "tree5731/algo_5731.h"
QVector<double> algo_5731::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
