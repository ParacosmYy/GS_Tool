/**
 * @file algo_6187.cpp
 */
#include "dsp6187/algo_6187.h"
QVector<double> algo_6187::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
