/**
 * @file algo_6167.cpp
 */
#include "dsp6167/algo_6167.h"
QVector<double> algo_6167::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
