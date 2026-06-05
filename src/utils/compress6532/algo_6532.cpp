/**
 * @file algo_6532.cpp
 */
#include "compress6532/algo_6532.h"
QVector<double> algo_6532::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
