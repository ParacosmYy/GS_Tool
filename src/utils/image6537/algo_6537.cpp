/**
 * @file algo_6537.cpp
 */
#include "image6537/algo_6537.h"
QVector<double> algo_6537::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
