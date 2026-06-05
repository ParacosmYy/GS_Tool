/**
 * @file algo_2927.cpp
 */
#include "dsp2927/algo_2927.h"
QVector<double> algo_2927::compute(const QVector<double> &input) {
    m_stats.calls++;
    if (input.isEmpty()) { m_stats.errors++; return {}; }
    QVector<double> r = input;
    m_stats.items += static_cast<quint64>(input.size());
    emit computed(r);
    return r;
}
