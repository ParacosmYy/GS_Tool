/**
 * @file algo_6752.cpp
 */
#include "compress6752/algo_6752.h"
QVector<double> algo_6752::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
