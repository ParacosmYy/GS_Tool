/**
 * @file algo_6807.cpp
 */
#include "dsp6807/algo_6807.h"
QVector<double> algo_6807::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
