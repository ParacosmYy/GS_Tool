/**
 * @file algo_6137.cpp
 */
#include "image6137/algo_6137.h"
QVector<double> algo_6137::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
