/**
 * @file algo_6312.cpp
 */
#include "compress6312/algo_6312.h"
QVector<double> algo_6312::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
