/**
 * @file algo_7532.cpp
 */
#include "compress7532/algo_7532.h"
QVector<double> algo_7532::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
