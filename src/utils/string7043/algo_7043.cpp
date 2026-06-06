/**
 * @file algo_7043.cpp
 */
#include "string7043/algo_7043.h"
QVector<double> algo_7043::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
