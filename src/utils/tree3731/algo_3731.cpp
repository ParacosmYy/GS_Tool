/**
 * @file algo_3731.cpp
 */
#include "tree3731/algo_3731.h"
QVector<double> algo_3731::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
