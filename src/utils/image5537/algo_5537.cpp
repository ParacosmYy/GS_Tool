/**
 * @file algo_5537.cpp
 */
#include "image5537/algo_5537.h"
QVector<double> algo_5537::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
